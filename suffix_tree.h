#pragma once
#include "helpers.h"
#include "lca_algo.h"

/* --------------------- SUFFIX ARRAY ----------------------- */
typedef struct _suffixArray
{
    int n;
    int *lcp, *a;
    
    // optional - only for even and odd tree
    int *suffixToRank;
} SuffixArray;


SuffixArray *CreateSuffixArray(int *lcp, int *a, int n);
void FreeSuffixArray(SuffixArray *sa);

/* ---------------------- SUFFIX TREE ------------------------ */
typedef struct _suffixTreeNode
{
    // depth - depth of node
    // from = suffixIndex + parentDepth
    // leaf (if != -1) + depth === length of string
    
    int parent, from, depth, leaf;
    
    int *children;
    int childrenCount;
} SuffixTreeNode;

typedef struct _suffixTree
{
    int count, capacity, leavesCount;
    SuffixTreeNode *nodes;
    
    // optional
    SuffixArray *suffixArray;
} SuffixTree;

SuffixTree *CreateSuffixTree(int capacity, int withRoot);
void FreeSuffixTree(SuffixTree *st);

int AllocateNextNodeIndexInSuffixTree(SuffixTree *st);
int GetParentDepth(SuffixTree *st, int i);
int GetSuffixForNode(SuffixTree *st, int i);
int CopyChildToSuffixTree(SuffixTree *st, int parent, int childOrder, SuffixTreeNode *node);
int AppendChildToSuffixTreeNode(SuffixTree *st, int parent, int childOrder, int from, int depth, int leaf, int childrenCount, int *children);

int AppendSubtreeToSuffixTree(SuffixTree *st, int parent, int childOrder, SuffixTree *srcTree, int srcSubTreeRoot, int rootFrom);
void BreakSuffixTreeEdgeByCustomLength(SuffixTree *tree, int currentNodeIndex, int childOrderNumber, int edgeLen);

void AddChildToBufferAndMakeItCurrent(DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer, int j);

int IsSubstring(SuffixTree *st, int *pattern, int *s, int n);

/* ---------------------- CONVERTERS ------------------------ */
SuffixTree *CreateSuffixTreeFromSuffixArray(SuffixArray *sa, int strLen);
SuffixArray *CreateSuffixArrayFromSuffixTree(SuffixTree *st);

/* ----------- EULER TOUR FOR SUFFIX ARRAY ------------------ */
typedef struct _suffixTreeEulerTour
{
    int n;
    DynamicArray *dfsDepths, *dfsToNode;
    int *rankToDfs;
    
    // external links
    SuffixTree *tree;
    int *suffixToRank;
} SuffixTreeEulerTour;

SuffixTreeEulerTour *GetSuffixTreeEulerTour(SuffixTree *st);
void FreeSuffixTreeEulerTour(SuffixTreeEulerTour *eulerTour);
int GetLcpForSuffixTree(LcaTable *lcaTable, SuffixTreeEulerTour *eulerTour, int v1, int v2);