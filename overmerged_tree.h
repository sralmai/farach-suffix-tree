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
    
    int linkToResultNode;
    int evenSuffix, oddSuffix, realDepth;
} OverMergedTreeNode;

typedef struct _overMegedTree
{
    int count, capacity;
    OverMergedTreeNode *nodes;
    
    OverMergedTreeEulerTour *eulerTour;
} OverMergedTree;

OverMergedTree *CreateOverMergedTree();
void FreeOverMergedTree(OverMergedTree *omt);

int AllocateNextNodeIndexInOverMergedTree(OverMergedTree *omt);
int AppendCustomNodeToOverMergedTree(OverMergedTree *omt, int parent, int depth, int evenIndex, int oddIndex);
int AppendNodeToOverMergedTree(OverMergedTree *omt, int parent, DfsPosition *px, DfsPosition *py);

void SetSuffixesToDfsByPosition(DfsPosition *p, int *suffixToDfs, int dfsIndex);
void SetSuffixesToDfs(SuffixTree *st, int parent, int childIndex, SuffixArray *sa, int *suffixToDfs, int dfsIndex, int *lastDfsLeaf);

OverMergedTree *OverMergeTrees(SuffixTree *evenTree, SuffixTree *oddTree, int *s, int n);
void SetSuffixesForLcaProblem(OverMergedTree *omt, int ind, SuffixTree *evenTree, SuffixTree *oddTree);

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

void BuildLcpTreeOnOverMergedTree(OverMergedTree *omt, OverMergedTreeEulerTour *eulerTour, SuffixTree *evenTree, SuffixTree *oddTree);
SuffixTree *BuildSuffixTreeFromOverMergedTree(OverMergedTree *omt, SuffixTree *evenTree, SuffixTree *oddTree, int *s, int n);
int GetLcaForOverMergedTree(LcaTable *lcaTable, OverMergedTreeEulerTour *eulerTour, int s1, int s2);