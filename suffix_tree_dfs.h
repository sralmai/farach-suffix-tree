#pragma once
#include "helpers.h"
#include "suffix_tree.h"

typedef struct _dfsPosition
{
    SuffixTree *tree;
    int treeType;
    //string
    int *s;
    
    //current node
    int ind;
    SuffixTreeNode *node;
    
    //children indexes in downward path
    DynamicArray *lastChild;
} DfsPosition;

#include <stdlib.h>
#include "suffix_tree_dfs.h"
#include "helpers.h"

DfsPosition *CreateDfsPosition(SuffixTree *st, int *s, int ind, int treeType);
void FreeDfsPosition(DfsPosition *p);

int EndOfDfs(DfsPosition *p);
int GetEdgeLength(DfsPosition *p);
int GetFirstCharOfChildEdge(DfsPosition *p);
int GetChildIndex(DfsPosition *p);

int CompareAndSwapDfsPositions(DfsPosition **ppx, DfsPosition **ppy);
int NextStepOfDfs(DfsPosition *p, int minDepth);
// int AppendNodeToSuffixTree(SuffixTree *st, int parent, DfsPosition *p, int copyChildren);

// int CopyBranchToSuffixTree(SuffixTree *st, int i, DfsPosition *p);
// void InitializeNextNodeInSuffixTree(SuffixTree *st, int parent, int suffixIndex, int depth, int leaf);
// void CompleteSuffixTreeNodeConstruction(SuffixTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer);
// void CopyChildrenToNode(SuffixTreeNode *node, int *children, int n);
// int AppendNodeToSuffixTree(SuffixTree *st, int parent, DfsPosition *p, int copyChildren);
void BreakSuffixTreeEdgeByCustomLength(SuffixTree *tree, int currentNodeIndex, int childOrderNumber, int edgeLen);
