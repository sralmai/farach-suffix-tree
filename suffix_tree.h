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

typedef struct _eulerTour
{
    int n;
    DynamicArray *dfsDepths;
    int *rankToDfs, suffixToRank;
} EulerTour;