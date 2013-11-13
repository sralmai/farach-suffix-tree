#pragma once

typedef struct _suffixTreeUnit
{
    int parent;
} SuffixTreeUnit;

typedef struct _suffixTree
{
    int n;
    int *lcp;
    int *a;
} SuffixTree;

SuffixTree *GetDegenerateTree();
void FreeTree(SuffixTree *tree);
SuffixTree *GetOddTree(int *s, int n);
SuffixTree *GetEvenTree(int *s, int n, SuffixTree *oddTree);
SuffixTree *BuildSuffixTree(int *s, int n);