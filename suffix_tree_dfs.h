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
    
    //children indexes in downward path
    DynamicArray *lastChild;
    
    // last dfs leaf node
    int lastDfsLeaf;
} DfsPosition;

DfsPosition *CreateDfsPosition(SuffixTree *st, int *s, int ind, int treeType);
void FreeDfsPosition(DfsPosition *p);

int EndOfDfs(DfsPosition *p);
int GetEdgeLength(DfsPosition *p);
int GetFirstCharOfChildEdge(DfsPosition *p);
int GetChildIndex(DfsPosition *p);

void SwapPositions(DfsPosition **ppx, DfsPosition **ppy);
int CompareAndSwapDfsPositions(DfsPosition **ppx, DfsPosition **ppy);
int NextStepOfDfs(DfsPosition *p, int minDepth);
