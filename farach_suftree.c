#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include "helpers.h"
#include "prefix_tree.h"
#include "suffix_tree.h"
#include "farach_suftree.h"
#include "lca_algo.h"

/* Variables, constants, methods */
const int DigitCapacity = 16;
int UniqueLetter = 0;

PrefixTree alphabetMapping;
int inputStringLength;
int *gtempArray;

/* -------------- Main logic ---------------- */
int main(int argc, char **argv)
{
    gtempArray = NULL;

    int res = 0;
    
    if (argc < 2) 
    {
        printf("name of input file hasn't been passed");
        res = 1;
        goto out;
    }
        
    Initialize(argv[1]);
    
    if (argc < 3)
    {
        printf("there's no file with tests");
        res = 1;
        goto cleanup;
    }
    
    TestSuffixTree(argv[2]);
    
cleanup:
    FreePrefixTree(&alphabetMapping);

out:
    return res;
}

void Initialize(const char *inputFileName)
{
    char *stemmedOutputFileName = ConcatStrings(inputFileName, "_stemmed");
    char *transformedOutputFileName = ConcatStrings(inputFileName, "_transformed");
    
    FILE *ptrFileInput = fopen(inputFileName, "r");
    FILE *ptrFileOutput = fopen(stemmedOutputFileName, "w");    
    
    CreateAlphabetMapping(ptrFileInput, ptrFileOutput);
    
    int rank = 0;
    fclose(ptrFileInput);
    fclose(ptrFileOutput);
    
    CompleteBuildingPrefixTree(&alphabetMapping, 0, &rank);
    UniqueLetter = rank;
    
    ptrFileInput = fopen(stemmedOutputFileName, "r");
    ptrFileOutput = fopen(transformedOutputFileName, "w");
    
    int *inputString = TransformInputString(ptrFileInput, ptrFileOutput);
    fclose(ptrFileInput);
    fclose(ptrFileOutput);
    
    MemFree(stemmedOutputFileName);
    MemFree(transformedOutputFileName);
    
    BuildSuffixTree(inputString, inputStringLength);
    
    MemFree(inputString);
    MemFree(gtempArray);
}

void CreateAlphabetMapping(FILE *ptrFileInput, FILE *ptrFileOutput)
{
    char *buffer = malloc(DigitCapacity);
    InitializePrefixTree(&alphabetMapping, 1);
        
    int c, len;
    inputStringLength = c = len = 0;
    
    while (1)
    {        
        c = toupper(fgetc(ptrFileInput));
        
        if (isalpha(c))
        {
            if (len < DigitCapacity)
            {
                putc(c, ptrFileOutput);
                buffer[len ++] = c;
            }
        }
        else
        {
            if (len > 0)
            {
                AddLetterToPrefixTree(&alphabetMapping, buffer, len);
                ++ inputStringLength;
                len = 0;
                putc('\n', ptrFileOutput);
            }
        }
        
        if (EOF == c)
            break;
    }    
    printf("number of prefix tree nodes: %d\n", alphabetMapping.count);    
    MemFree(buffer);    
}

int *TransformInputString(FILE *ptrFileInput, FILE *ptrFileOutput)
{
    printf("transformed input string have %d letters\n", inputStringLength);
    
    char *buffer = malloc(DigitCapacity);
    
    int *inputString = malloc((inputStringLength + 1) * sizeof *inputString);
    int i, c, letter, len;
    len = i = 0;
    
    while (1)
    {      
        c = fgetc(ptrFileInput);
        if (isalpha(c))
        {
            if (len < DigitCapacity)
                buffer[len ++] = c;
        }
        else
        {
            if (len > 0)
            {
                letter = GetIndexOfLetterInPrefixTree(&alphabetMapping, buffer, len);
                fprintf(ptrFileOutput, "%d\n", letter);
                
                inputString[i ++] = letter;
                len = 0;
            }
        }
        
        if (EOF == c)
            break;
    }
    
    MemFree(buffer);
    
    // add unique symbol at the end of the string. It has to be greater than the others
    inputString[inputStringLength] = UniqueLetter;
    return inputString;
}

// in Farach algorithm it's named "building an odd tree" (because of indexing from 1)
SuffixTree *GetEvenSuffixTree(int *s, int n)
{
    int m = (n + 1) / 2;
    if (m == 1)
        return CreateSuffixTree();
    
    int *evenS = malloc(m * sizeof *evenS);
    for (int i = 0; i < m; ++i)
        evenS[i] = 2 * i;        
    RadixSort(evenS, m, s, 1);
    RadixSort(evenS, m, s, 0);
    debugArr(evenS, m);
    
    int k = 0;
    int *newS = malloc((m + 1) * sizeof *newS);    
    for (int i = 0; i < m; ++i)
    {
        if (i > 0 && (s[evenS[i]] != s[evenS[i - 1]] || s[evenS[i] + 1] != s[evenS[i - 1] + 1]))
            ++k;
        newS[evenS[i] / 2] = k;
    }
    newS[m] = k;    
    debugArr(newS, m + 1);
    
    SuffixTree *evenSubTree = BuildSuffixTreeByFarachAlgorithm(newS, m);
    SuffixArray *evenSubArray = CreateSuffixArrayFromSuffixTree(evenSubTree, ..);
    
    //no need to malloc a and lcp - just use already allocated arrays
    int *a = newS, 
        *lcp = lcp, 
        *subA = evenSubTree->a, 
        *subLcp = evenSubTree->lcp;
    
    for (int i = 0; i < m; i++)
        a[i] = 2 * subA[i];
    
    for (int i = 0; i < m - 1; i++)
    {
        lcp[i] = 2 * subLcp[i];
        
        if (a[i] + 2 * subLcp[i] > n || a[i + 1] + 2 * subLcp[i] > n)
        {
            printf("CHECK borders: %d\n", i);            
            fflush(stdout);
        }
        else 
        
        if (s[a[i] + 2 * subLcp[i]] == s[a[i + 1] + 2 * subLcp[i]])
            ++ lcp[i];
    }
    lcp[m - 1] = 0;
    
    SuffixArray *evenArray = CreateSuffixArray(lcp, a, m);
    SuffixTree *evenTree = CreateSuffixTreeFromSuffixArray(evenArray);
    evenTree->suffixArray = evenArray;
    
    debug("evenTree");
    debugArr(a, m);
    debugArr(lcp, m - 1);
    
    FreeSuffixTree(evenSubTree);
    FreeSuffixArray(evenSubArray);
    
    return evenTree;
}

// in Farach algorithm it's named "building an even tree" (because of indexing from 1)
SuffixTree *GetOddSuffixTree(int *s, int n, SuffixTree *evenTree)
{
    int m = n / 2;
    if (m < 2)
        return CreateSuffixTree();
    
    int *oddS = malloc(m * sizeof *oddS),
        *evenA = evenTree->suffixArray->a, 
        *evenLcp = evenTree->suffixArray->lcp;
        
    for (int i = 0, j = 0; i < m; i++)
    {
        if (evenA[i] > 0)
            oddS[j++] = evenA[i] - 1;
    }
    if (n % 2 == 0)
        oddS[m - 1] = n - 1;
        
    RadixSort(oddS, m, s, 0);
    debugArr(oddS, m);
        
    int *a = malloc(m * sizeof *oddArray->a), 
        *lcp = malloc(m * sizeof *oddArray->lcp);
    
    for (int i = 0; i < m; i++)
        a[i] = oddS[i];    
        
    
    int evenN = evenTree->suffixArray->n;
    int *suffixToRank = malloc(evenN * sizeof *suffixToRank);
    for (int i = 0; i < evenN; i++)
        suffixToRank[evenA[i] / 2] = i;
    
    SuffixArrayEulerTour *eulerTour = GetSuffixArrayEulerTour(evenTree->suffixArray, suffixToRank);        
    LcaTable *lcaTable = CreateLcaTable(eulerTour->dfsDepths);
        
    for (int i = 0; i < m - 1; i++)
    {
        if (s[a[i]] == s[a[i + 1]])
            lcp[i] = GetLcpForSuffixArray(lcaTable, eulerTour, (a[i] + 1)/2, (a[i + 1] + 1)/2) + 1;
        else
            lcp[i] = 0;
    }        
    lcp[m - 1] = 0;
        
    SuffixArray *oddArray = CreateSuffixArray(lcp, a, m);
    SuffixTree *oddTree = CreateSuffixTreeFromSuffixArray(oddArray);
    oddTree->suffixArray = oddArray;
    
    debug("oddTree");
    debugArr(lcp, m - 1);
    
    MemFree(oddS);
    FreeLcaTable(lcaTable);
    FreeSuffixArrayEulerTour(eulerTour);
    
    return oddTree;
}

SuffixTree *BuildSuffixTreeByFarachAlgorithm(int *s, int n)
{
    if (n > 10)
    {
    int x[13] = {1, 2, 1, 1, 1, 2, 2, 1, 2, 2, 2, 1, 3};
    SuffixTree *oddTree = GetOddTree(x, 12);
    SuffixTree *evenTree = GetEvenTree(x, 12, oddTree);
    
//    return MergeOddAndEvenTrees(s, n, oddTree, evenTree);

    FreeTree(oddTree);
    FreeTree(evenTree);
    }
    
    SuffixTree *tree = calloc(1, sizeof *tree);
    tree->n = 12;
    tree->a = malloc(7 * sizeof *tree->a);
    tree->lcp = malloc(7 * sizeof *tree->lcp);
    int *a = tree->a, *lcp = tree->lcp;
    a[0] = 1; a[1] = 0; a[2] = 2; a[3] = 3; a[4] = 5; a[5] = 4; a[6] = 6;
    lcp[0] = 0; lcp[1] = 1; lcp[2] = 0; lcp[3] = 1; lcp[4] = 0; lcp[5] = 0, lcp[6] = 0;
    
    return tree;
}

SuffixTree *BuildSuffixTreeByFarachAlgorithm_Release(int *s, int n)
{
    SuffixTree *evenTree = GetEvenSuffixTree(s, n);
    SuffixTree *oddTree = GetOddSuffixTree(s, n, evenTree);
    
    OverMergedTree *omt = OverMergeTrees(evenTree, oddTree, s, n);
    SuffixTree *st = BuildSuffixTreeFromOverMergedTree(omt);
    
    FreeSuffixTree(evenTree);
    FreeSuffixTree(oddTree);
    FreeOverMergedTree(omt);
    
    return st;
}

void TestSuffixTree(const char *testsInputFileName)
{
    FILE *ptrFileInput = fopen(testsInputFileName, "r");     
    
    char *buffer = malloc(DigitCapacity);
    DynamicArray *testString = CreateDynamicArray(1);
    
    int c, len;
    len = 0;
    
    while (1)
    {
        c = toupper(fgetc(ptrFileInput));
        
        if (isalpha(c))
        {
            if (len < DigitCapacity)
                buffer[len ++] = c;
        }
        else
        {
            if (len > 0)
            {
                PushToDynamicArray(testString, GetIndexOfLetterInPrefixTree(&alphabetMapping, buffer, len));
                len = 0;
            }
            
            if (('$' == c || EOF == c) && testString->count > 0)
            {
                // TODO
                testString->count = 0;
                
                debug("OK");
            }
        }
        
        if (EOF == c)
            break;
    }
    
    MemFree(buffer);
    FreeDynamicArray(testString);
    
    fclose(ptrFileInput);
}

void RadixSort(int *a, int n, int *s, int j)
{   
    int i, maxA = s[a[0]], exp = 1;
    if (!gtempArray)
        gtempArray = malloc(n * sizeof *gtempArray);
            
    for (i = 1; i < n; i++)
    {
        if (s[a[i] + j] > maxA)
            maxA = s[a[i] + j];
    }

    while (maxA / exp > 0)
    {
        int bucket[10] = { 0 };
        
        for (i = 0; i < n; i++)
            bucket[(s[a[i] + j] / exp) % 10]++;            
        for (i = 1; i < 10; i++)
            bucket[i] += bucket[i - 1];
        for (i = n - 1; i >= 0; i--)
            gtempArray[--bucket[(s[a[i] + j] / exp) % 10]] = a[i];
        for (i = 0; i < n; i++)
            a[i] = gtempArray[i];
        exp *= 10;
    }
}
