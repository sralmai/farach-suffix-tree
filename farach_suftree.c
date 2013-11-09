#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "farach_suftree.h"

/* Variables, constants, methods */
const int DigitCapacity = 16;
const int UniqueLetter = 0;

PrefixTree alphabetMapping;
int inputStringLength;
int *gtempArray, *oddS, *evenS;

void Initialize(const char *inputFileName);
void TestSuffixTree(const char *testsInputFileName);

/* -------------- Main logic ---------------- */
int main(int argc, char **argv)
{
    gtempArray = oddS = evenS = NULL;

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

void debug(const char *s)
{
    printf("==> %s\n", s);
    fflush(stdout);
}

void CreateAlphabetMapping(FILE *ptrFileInput, FILE *ptrFileOutput);
int *TransformInputString(FILE *ptrFileInput, FILE *ptrFileOutput);
SuffixTree *BuildSuffixTree(int *s, int n);

void Initialize(const char *inputFileName)
{
    const char stemmedOutputFileNameSuffix[] = "_stemmed";
    const char transformedOutputFileNameSuffix[] = "_transformed";
    char *stemmedOutputFileName = NULL;
    char *transformedOutputFileName = NULL;
    int len = strlen(inputFileName);
    
    stemmedOutputFileName = malloc(len + strlen(stemmedOutputFileNameSuffix) + 1);
    strcpy(stemmedOutputFileName, inputFileName);
    strcpy(stemmedOutputFileName + len, stemmedOutputFileNameSuffix);
    
    transformedOutputFileName = malloc(len + strlen(transformedOutputFileNameSuffix) + 1);
    strcpy(transformedOutputFileName, inputFileName);
    strcpy(transformedOutputFileName + len, transformedOutputFileNameSuffix);
    
    FILE *ptrFileInput = fopen(inputFileName, "r");
    FILE *ptrFileOutput = fopen(stemmedOutputFileName, "w");    
    
    CreateAlphabetMapping(ptrFileInput, ptrFileOutput);
    
    int rank = UniqueLetter + 1;
    fclose(ptrFileInput);
    fclose(ptrFileOutput);
    
    CompletePrefixTreeBuild(&alphabetMapping, 0, &rank);
    
    ptrFileInput = fopen(stemmedOutputFileName, "r");
    ptrFileOutput = fopen(transformedOutputFileName, "w");
    
    int *inputString = TransformInputString(ptrFileInput, ptrFileOutput);
    fclose(ptrFileInput);
    fclose(ptrFileOutput);
    
    free(stemmedOutputFileName);
    free(transformedOutputFileName);
    
    BuildSuffixTree(inputString, inputStringLength);
    if (gtempArray) free(gtempArray);
    
    free(inputString);
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
                inputStringLength++;
                len = 0;
                putc('\n', ptrFileOutput);
            }
        }
        
        if (EOF == c)
            break;
    }    
    printf("number of prefix tree nodes: %d\n", alphabetMapping.count);    
    free(buffer);    
}

int *TransformInputString(FILE *ptrFileInput, FILE *ptrFileOutput)
{
    printf("transformed input string have %d letters\n", inputStringLength);
    
    char *buffer = malloc(DigitCapacity);
    int len, i;
    len = 0;
    i = inputStringLength;
    
    while (i > 0)
    {
        i = i >> 1;
        len ++;
    }
    len++;
    
    int *inputString = malloc((1 << len) * sizeof *inputString);
    int c, letter;
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
                letter = GetIndexOfLetter(&alphabetMapping, buffer, len);
                fprintf(ptrFileOutput, "%d\n", letter);
                
                inputString[i++] = letter;
                len = 0;
            }
        }
        
        if (EOF == c)
            break;
    }
    
    free(buffer);
    
    // add unique symbol at the end of the string. It has to be less than the others
    inputString[i] = UniqueLetter;
    return inputString;
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

void BuildTransformedOddS(int *s, int n)
{
    int m = n / 2;
    if (!oddS)
        oddS = malloc(m * sizeof *oddS);
    
    for (int i = 0; i < m; i++)
        oddS[i] = 2 * i;
        
    RadixSort(oddS, m, s, 0);
    RadixSort(oddS, m, s, 1);
    
    return oddS;
}

SuffixTree *GetOddTree(int *s, int n)
{
    int m = n / 2;
    BuildTransformedOddS(s, n);
    SuffixTree *oddSubTree = BuildSuffixTree(oddS, m);
    
    SuffixTree *oddTree = calloc(1, sizeof *oddTree);
    oddTree->a = malloc(m * sizeof *oddTree->a);
    oddTree->lcp = malloc(m * sizeof *oddTree->lcp);
    
    int *a = oddTree->a, 
        *lcp = oddTree->lcp, 
        *subA = oddSubTree->a, 
        *subLcp = oddSubTree->lcp;
    
    for (int i = 0; i < m; i++)
    {
        a[i] = 2 * subA[i] - 1;
        lcp[i] = 2 * subLcp[i];
        if (s[a[i] + 2 * subLcp[i] == s[a[i + 1] + 2 * subLcp[i])
            lcp[i]++;
    }
}

SuffixTree *GetEvenTree(int *s, int n, SuffixTree *oddTree)
{
    int m = n / 2;
    int *evenS = GetTransformedEvenS(s, n, oddTree);
    SuffixTree *evenSubTree = BuildSuffixTree(evenS, m);
    
    SuffixTree *evenTree = calloc(1, sizeof *evenTree);
    evenTree->a = malloc(m * sizeof *evenTree->a);
    evenTree->lcp = malloc(m * sizeof *evenTree->lcp);
    
    int *a = evenTree->a, 
        *lcp = evenTree->lcp, 
        *subA = evenSubTree->a, 
        *subLcp = evenSubTree->lcp;
    
    for (int i = 0; i < m; i++)
    {
        a[i] = 2 * subA[i] - 1;
        lcp[i] = 2 * subLcp[i];
        if (s[a[i] + 2 * subLcp[i] == s[a[i + 1] + 2 * subLcp[i])
            lcp[i]++;
    }    
}

SuffixTree *BuildSuffixTree(int *s, int n)
{
    SuffixTree *oddTree = GetOddTree(s, n);
    SuffixTree *evenTree = GetEvenTree(s, n, oddTree);
    
    return MergeOddAndEvenTrees(s, n, oddTree, evenTree);
}

void ResizeArray(int **arr, int *len)
{
    int *t = realloc(*arr, *len * 2 * sizeof *t);
    if (t)
    {
        *arr = t;
        *len = *len << 1;
    }
}

void TestSuffixTree(const char *testsInputFileName)
{
    FILE *ptrFileInput = fopen(testsInputFileName, "r");     
    
    char *buffer = malloc(DigitCapacity);
    int arrayLength = 1;
    int *testString = NULL;
    ResizeArray(&testString, &arrayLength);
    
    int c, letter, len, i;
    len = i = 0;
    
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
                letter = GetIndexOfLetter(&alphabetMapping, buffer, len);
                
                if (i == arrayLength)
                    ResizeArray(&testString, &arrayLength);
                
                testString[i++] = letter;                
                len = 0;
            }
            
            if (('$' == c || EOF == c) && i > 0)
            {
                // TODO
                i = 0;
                
                debug("OK");
            }
        }
        
        if (EOF == c)
            break;
    }
    
    free(buffer);
    free(testString);
    
    fclose(ptrFileInput);
}

/* ----------- Prefix tree -------------- */
void InitializePrefixTree(PrefixTree *pTree, int size)
{
    debug("initialize prefix tree");
    
    pTree->count = 0;
    pTree->arrLength = size;
    pTree->units = NULL;
    
    pTree->units = malloc(pTree->arrLength * sizeof *pTree->units);
    if (!pTree->units)
        debug("bad malloc in InitializePrefixTree");
    
    printf("struct size: %d\n", (int)sizeof *pTree->units);
    
    GetNextPrefixTreeUnitIndex(pTree);
}

void FreePrefixTree(PrefixTree *pTree)
{
    debug("free prefix tree");
    
    pTree->count = 0;
    pTree->arrLength = 0;
    if (pTree->units) free(pTree->units);
}

int GetNextPrefixTreeUnitIndex(PrefixTree *pTree)
{    
    int i = pTree->count;
    if (i == pTree->arrLength)
    {
        PrefixTreeUnit *old = NULL;
        debug("resize array of units");        
        printf("new size: %d bytes\n", (int) (pTree->arrLength * 2 * sizeof *old));

        old = realloc(pTree->units, 2 * pTree->arrLength * sizeof *pTree->units);
        if (!old)
            return -1;
        
        pTree->units = old;            
        pTree->arrLength = pTree->arrLength * 2;        
    }
    
    pTree->units[i].rank = -1;
    for (int j = 0; j < EngAlphabetCardinality; j++)
        pTree->units[i].children[j] = -1;        
    pTree->count++;
    
    return i;
}

void AddLetterToPrefixTree(PrefixTree *pTree, char *s, int len)
{    
    int ind = 0;
    int childUnitIndex = -1;
    int c;
    
    for (int i = 0; i < len; i++)
    {
        c = s[i] - 'A';
        if (c < 0)
        {
            debug("c < 0");
            return;
        }
        
        childUnitIndex = pTree->units[ind].children[c];
        if (childUnitIndex < 0)
        {
            childUnitIndex = GetNextPrefixTreeUnitIndex(pTree);
            if (childUnitIndex < 0)
            {
                debug("childUnitIndex < 0");
                return;
            }
            
            pTree->units[ind].children[c] = childUnitIndex;
        }
        ind = childUnitIndex;
    }
    
    pTree->units[ind].rank = 1;
}

void CompletePrefixTreeBuild(PrefixTree *pTree, int i, int *rank)
{    
    if (1 == pTree->units[i].rank)
        pTree->units[i].rank = (*rank)++;
        
    for (int j = 0; j < EngAlphabetCardinality; j++)
    {
        if (pTree->units[i].children[j] >= 0)
            CompletePrefixTreeBuild(pTree, pTree->units[i].children[j], rank);
    }
}

int GetIndexOfLetter(PrefixTree *pTree, char *s, int len)
{
    int c, ind = 0;
    
    for (int i = 0; i < len; i++)
    {
        c = s[i] - 'A';
        if (pTree->units[ind].children[c] >= 0)
            ind = pTree->units[ind].children[c];
        else
            return -1;
    }
    return pTree->units[ind].rank;
}