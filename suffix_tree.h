#pragma once

typedef struct _suffixTreeNode
{
    int parent, from, depth, leaf;    
    int *children;
} SuffixTreeNode;

typedef struct _suffixTree
{
    int count, capacity;
    SuffixTreeNode *nodes;
} SuffixTree;

typedef struct _suffixArray
{
    int n;
    int *lcp, *a;
} SuffixArray;

SuffixArray *GetDegenerateSuffixArray();
void FreeSuffixArray(SuffixArray *sa);

SuffixTree *GetDegenerateSuffixTree();
void FreeSuffixTree(SuffixTree *st);