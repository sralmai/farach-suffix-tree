#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "farach_suftree.h"

/* Variables, constants, methods */
const int DigitCapacity = 16;
PrefixTree alphabetMapping;
int inputStringLength;

void Initialize(const char *inputFileName);
void TestSuffixTree(const char *testsInputFileName);

/* -------------- Main logic ---------------- */
int main(int argc, char **argv)
{
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
void BuildSuffixTree();

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
    
    int rank = 0;
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
    
    BuildSuffixTree(inputString);
    
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
    int *inputString = malloc(inputStringLength * sizeof *inputString + 1);
    int c, letter, len, i;
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
    
    // add unique symbol "-1" at the end of the string
    inputString[i] = -1;
    return inputString;
}

void BuildSuffixTree()
{
    //TODO
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