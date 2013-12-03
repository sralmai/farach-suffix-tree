#pragma once
#include "suffix_tree.h"
#include "helpers.h"
#include "suffix_tree_dfs.h"

typedef struct _overMergedTreeEulerTour OverMergedTreeEulerTour;

/* ----------------------- OVERMERGED SUFFIX TREE ----------------------- */
typedef struct _overMegedTreeNode
{
    int parent;
    int oddIndex, evenIndex;
    
    int *children;
    int childrenCount;
    int depth;
    
    SuffixTree *evenTree, *oddTree;
} OverMergedTreeNode;

typedef struct _overMegedTree
{
    int count, capacity;
    OverMergedTreeNode *nodes;
    
    OverMergedTreeEulerTour *eulerTour;
    
    SuffixTree *evenTree, *oddTree;
} OverMergedTree;

OverMergedTree *CreateOverMergedTree();
void FreeOverMergedTree(OverMergedTree *omt);
int AllocateNextNodeIndexInOverMergedTree(OverMergedTree *omt);
int AppendNodeToOverMergedTree(OverMergedTree *omt, int parent, DfsPosition *px, DfsPosition *py);
void SetSuffixesToDfs(SuffixTree *st, int ind, SuffixArray *sa, int *suffixToDfs, int dfsIndex, int *leftChild);
OverMergedTree *OverMergeTrees(SuffixTree *evenTree, SuffixTree *oddTree, int *s, int n);


/* ----------------- OVERMERGED SUFFIX TREE EULER TOUR ------------------ */
struct _overMergedTreeEulerTour
{
    int n;
    DynamicArray *dfsDepths, *dfsToNode;
    int *suffixToDfs;
    
    OverMergedTree *tree;
};

OverMergedTreeEulerTour *CreateOverMergedTreeEulerTour(int n, DynamicArray *dfsDepths, DynamicArray *dfsToNode, int *suffixToDfsDepths, OverMergedTree *tree);
void FreeOverMergedTreeEulerTour(OverMergedTreeEulerTour *eulerTour);
SuffixTree *BuildSuffixTreeFromOverMergedTree(OverMergedTree *omt, int *s, int n);
int GetLcaForOverMergedTree(LcaTable *lcaTable, OverMergedTreeEulerTour *eulerTour, int s1, int s2);