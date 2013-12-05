#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "helpers.h"
#include "prefix_tree.h"
#include "lca_algo.h"
#include "suffix_tree.h"
#include "overmerged_tree.h"
#include "farach_suftree.h"

/* Variables, constants, methods */
const int DigitCapacity = 16;
int UniqueLetter;
const char *SUBSTR_PRINT = "+", *NOT_SUBSTR_PRINT = "-";

PrefixTree alphabetMapping;
int *globalRadixSortBuffer, *inputString;
int inputStringLength;

SuffixTree *patternSuffixTree;

/* -------------- Main logic ---------------- */
int main(int argc, char **argv)
{
    DEBUG = 1;
    inputString = NULL;

    int res = 0;
    
    if (argc < 2) 
    {
        printf("name of input file hasn't been passed");
        res = 1;
        goto out;
    }
        
    Initialize(argv[1]);
    TestSuffixTree(patternSuffixTree, inputString, argv[1]);
    
    FreePrefixTree(&alphabetMapping);
    MemFree(inputString);
    FreeSuffixTree(patternSuffixTree);

out:
    return res;
}

void Initialize(const char *inputFileName)
{
    char *stemmedOutputFileName = ConcatStrings(inputFileName, "_stemmed");
    char *transformedOutputFileName = ConcatStrings(inputFileName, "_transformed");
    
    FILE *ptrFileInput = fopen(inputFileName, "r");
    FILE *ptrFileOutput = fopen(stemmedOutputFileName, "w");    
    
    inputStringLength = CreateAlphabetMapping(ptrFileInput, ptrFileOutput);
    
    int rank = 0;
    fclose(ptrFileInput);
    fclose(ptrFileOutput);
    
    CompleteBuildingPrefixTree(&alphabetMapping, 0, &rank);
    UniqueLetter = rank;
    
    ptrFileInput = fopen(stemmedOutputFileName, "r");
    ptrFileOutput = fopen(transformedOutputFileName, "w");
    
    inputString = TransformInputString(ptrFileInput, ptrFileOutput, inputStringLength);
    fclose(ptrFileInput);
    fclose(ptrFileOutput);
    
    MemFree(stemmedOutputFileName);
    MemFree(transformedOutputFileName);
    
    globalRadixSortBuffer = NULL;
    patternSuffixTree = BuildSuffixTreeByFarachAlgorithm(inputString, inputStringLength);

    debugInt(patternSuffixTree->count);
    MemFree(globalRadixSortBuffer);
}

int CreateAlphabetMapping(FILE *ptrFileInput, FILE *ptrFileOutput)
{
    char *buffer = malloc(DigitCapacity);
    InitializePrefixTree(&alphabetMapping, 1);
        
    int c, len;
    c = len = 0;
    int inputStringLength = 0;
    
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

    return inputStringLength;
}

int *TransformInputString(FILE *ptrFileInput, FILE *ptrFileOutput, int inputStringLength)
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
    
    int *evenS = malloc(m * sizeof *evenS);
    for (int i = 0; i < m; ++i)
        evenS[i] = 2 * i;        
    RadixSort(evenS, m, s, 1);
    RadixSort(evenS, m, s, 0);
    
    int k = 0;
    int *newS = malloc((m + 1) * sizeof *newS);    
    for (int i = 0; i < m; ++i)
    {
        if (i > 0 && (s[evenS[i]] != s[evenS[i - 1]] || s[evenS[i] + 1] != s[evenS[i - 1] + 1]))
            ++k;
        newS[evenS[i] / 2] = k;
    }
    newS[m] = ++k;
    
    SuffixTree *evenSubTree = BuildSuffixTreeByFarachAlgorithm(newS, m);
    SuffixArray *evenSubArray = CreateSuffixArrayFromSuffixTree(evenSubTree);
    
    //no need to malloc a and lcp - just use already allocated arrays
    int *a = newS, 
        *lcp = evenS, 
        *subA = evenSubArray->a, 
        *subLcp = evenSubArray->lcp;
    
    for (int i = 0; i < m; i++)
        a[i] = 2 * subA[i];
    a[m] = n;
    debugArr(a, m);
    
    for (int i = 0; i < m - 1; i++)
    {
        lcp[i] = 2 * subLcp[i];        
        if (s[a[i] + 2 * subLcp[i]] == s[a[i + 1] + 2 * subLcp[i]])
            ++ lcp[i];
    }
    lcp[m - 1] = 0;
    debugArr(lcp, m - 1);
    debugPrint("=======\n");
    
    SuffixArray *evenArray = CreateSuffixArray(lcp, a, m);
    SuffixTree *evenTree = CreateSuffixTreeFromSuffixArray(evenArray, n);
        
    // free resources
    FreeSuffixTree(evenSubTree);
    FreeSuffixArray(evenSubArray);
    
    return evenTree;
}

// in Farach algorithm it's named "building an even tree" (because of indexing from 1)
SuffixTree *GetOddSuffixTree(int *s, int n, SuffixTree *evenTree)
{
    int m = n / 2;    
    int *oddS = malloc((m + 1) * sizeof *oddS),
        *evenA = evenTree->suffixArray->a;
        
    for (int i = 0, j = 0; i < evenTree->leavesCount; i++)
    {
        if (evenA[i] > 0)
            oddS[j++] = evenA[i] - 1;
    }
    if (n % 2 == 0)
        oddS[m - 1] = n - 1;
    else
        oddS[m] = n;
        
    RadixSort(oddS, m, s, 0);
    debugArr(oddS, m);
    debugPrint("~~~~~~~~\n");
        
    int *a = oddS, 
        *lcp = malloc(m * sizeof *lcp);
        
    int evenN = evenTree->suffixArray->n;
    int *suffixToRank = malloc(evenN * sizeof *suffixToRank);
    for (int i = 0; i < evenN; i++)
        suffixToRank[evenA[i] / 2] = i;
    
    SuffixTreeEulerTour *eulerTour = GetSuffixTreeEulerTour(evenTree, suffixToRank);        
    LcaTable *lcaTable = CreateLcaTable(eulerTour->dfsDepths, eulerTour->dfsToNode);
        
    for (int i = 0; i < m - 1; i++)
    {
        if (s[a[i]] == s[a[i + 1]])
            lcp[i] = GetLcpForSuffixTree(lcaTable, eulerTour, a[i] + 1, a[i + 1] + 1) + 1;
        else
            lcp[i] = 0;
    }        
    lcp[m - 1] = 0;
        
    SuffixArray *oddArray = CreateSuffixArray(lcp, a, m);
    SuffixTree *oddTree = CreateSuffixTreeFromSuffixArray(oddArray, n);
    
    // free resources
    FreeLcaTable(lcaTable);
    FreeSuffixTreeEulerTour(eulerTour);
    
    return oddTree;
}

SuffixTree *BuildSuffixTreeByFarachAlgorithm(int *s, int n)
{
    if (n == 0)
        return CreateSuffixTree(1);
    if (n == 1)
    {
        SuffixTree *st = CreateSuffixTree(0);
        // add root && child
        AppendChildToSuffixTreeNode(st, 0, -1, 0, 0, -1, 1, NULL);
        AppendChildToSuffixTreeNode(st, 0, 0, 0, 1, 0, 0, NULL);
        st->leavesCount = 1;
        
        return st;
    }
    
    SuffixTree *evenTree = GetEvenSuffixTree(s, n);
    SuffixTree *oddTree = GetOddSuffixTree(s, n, evenTree);
    
    OverMergedTree *omt = OverMergeTrees(evenTree, oddTree, s, n);
    SuffixTree *st = BuildSuffixTreeFromOverMergedTree(omt, evenTree, oddTree, s, n);
    
    FreeSuffixTree(evenTree);
    FreeSuffixTree(oddTree);
    FreeOverMergedTree(omt);
    
    return st;
}

void TestSuffixTree(SuffixTree *st, int *pattern, const char *inputFileName)
{
    char *testsForInputFileName = ConcatStrings(inputFileName, "tests");
    FILE *ptrFileInput = fopen(testsForInputFileName, "r");
    
    char *buffer = malloc(DigitCapacity);
    DynamicArray *testString = CreateDynamicArray(1);
    
    int c, len;
    len = 0;
    int testIndex = 0;
    
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
            
            if ('$' == c && testString->count > 0)
            {
                c = fgetc(ptrFileInput);
                const char *expected = (c == '1') ? SUBSTR_PRINT : NOT_SUBSTR_PRINT;                
                const char *result = IsSubstring(st, pattern, testString->a, testString->count) ? SUBSTR_PRINT : NOT_SUBSTR_PRINT;
                printf("Test%d: %s. \t| Expected: %s \t| Got: %s\n", testIndex, (result == expected) ? "OK" : "ERR", expected, result);
                
                testIndex ++;                    
                testString->count = 0;
            }
        }
        
        if (EOF == c)
            break;
    }
    
    // free resources
    MemFree(buffer);
    FreeDynamicArray(testString);
    
    fclose(ptrFileInput);
    MemFree(testsForInputFileName);
}

void RadixSort(int *a, int n, int *s, int j)
{   
    int i, maxA = s[a[0]], exp = 1;
    if (!globalRadixSortBuffer)
        globalRadixSortBuffer = malloc(n * sizeof *globalRadixSortBuffer);
            
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
            globalRadixSortBuffer[--bucket[(s[a[i] + j] / exp) % 10]] = a[i];
        for (i = 0; i < n; i++)
            a[i] = globalRadixSortBuffer[i];
        exp *= 10;
    }
}
