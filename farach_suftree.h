#pragma once
#include "suffix_tree.h"

void Initialize(const char *inputFileName);

int CreateAlphabetMapping(FILE *ptrFileInput, FILE *ptrFileOutput);
int *TransformInputString(FILE *ptrFileInput, FILE *ptrFileOutput, int inputStringLength);

// in Farach algorithm it's named "building an odd tree" (because of indexing from 1)
SuffixTree *GetEvenSuffixTree(int *s, int n);
// in Farach algorithm it's named "building an even tree" (because of indexing from 1)
SuffixTree *GetOddSuffixTree(int *s, int n, SuffixTree *evenTree);
SuffixTree *BuildSuffixTreeByFarachAlgorithm(int *s, int n);
SuffixTree *BuildSuffixTreeByFarachAlgorithm_Release(int *s, int n);
void TestSuffixTree(const char *testsInputFileName);
void RadixSort(int *a, int n, int *s, int j);