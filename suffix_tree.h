#pragma once

typedef struct _suffixTreeNode
{
    int parent, from, depth, leaf;
    
    int *children;
    int childrenCount;
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

SuffixArray *CreateSuffixArray();
void FreeSuffixArray(SuffixArray *sa);

SuffixTree *CreateSuffixTree();
void FreeSuffixTree(SuffixTree *st);