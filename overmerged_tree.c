#include <stdlib.h>
#include <stdio.h>
#include "overmerged_tree.h"


/* --------- internal methods ---------- */
OverMergedTree *CreateOverMergedTree();
/* ------------------------------------- */

OverMergedTree *CreateOverMergedTree()
{
    OverMergedTree *omt = calloc(1, sizeof *omt);
    omt->count = 0;
    omt->capacity = 1;
    omt->nodes = malloc(omt->capacity * sizeof *omt->nodes);
    
    if (!omt->nodes)
        debug("bad nodes malloc in OverMergeTrees");
            
    return omt;
}

void FreeOverMergedTree(OverMergedTree *omt)
{
    debug("free OverMergedTree: started");
    for (int i = 0; i < omt->count; i++)
        MemFree(omt->nodes[i].children);
    
    FreeOverMergedTreeEulerTour(omt->eulerTour);
    MemFree(omt->nodes);
    MemFree(omt);
    debug("free OverMergedTree: finished");
}

int AllocateNextNodeIndexInOverMergedTree(OverMergedTree *omt)
{    
    int i = omt->count;
    if (i == omt->capacity)
    {        
        OverMergedTreeNode *t = realloc(omt->nodes, 2 * omt->capacity * sizeof *omt->nodes);
        if (t)
        {
            omt->nodes = t;
            omt->capacity = omt->capacity << 1;
        }
        else
            debug("bad overmerged tree nodes realloc");
    }
    omt->nodes[i].children = NULL;
    ++omt->count;
    
    return i;
}

int AppendNodeToOverMergedTree(OverMergedTree *omt, int parent, DfsPosition *px, DfsPosition *py)
{    
    int nodeIndex = AllocateNextNodeIndexInOverMergedTree(omt);
    OverMergedTreeNode *node = &(omt->nodes[nodeIndex]);
    
    int srcNodeIndex = GetChildIndex(px);
    SuffixTreeNode *srcNode = &(px->tree->nodes[srcNodeIndex]);
    
    node->parent = parent;
    node->depth = srcNode->depth;
    
    if (0 == px->treeType)
    {
        node->evenIndex = srcNodeIndex;
        node->oddIndex = py ? GetChildIndex(py) : -1;
    }
    else
    {
        node->oddIndex = srcNodeIndex;
        node->evenIndex = py ? GetChildIndex(py) : -1;
    }
    return nodeIndex;
}

void SetSuffixesToDfs(SuffixTree *st, int ind, SuffixArray *sa, int *suffixToDfs, int dfsIndex, int *leftChild)
{    
    int rightSuffix = -1 == ind ? sa->n : st->nodes[ind].from - st->nodes[st->nodes[ind].parent].depth;
    
    while (sa->a[*leftChild] != rightSuffix)
    {
        suffixToDfs[sa->a[*leftChild]] = dfsIndex;
        ++(*leftChild);
    }
}

void CompleteOverMergedTreeNodeConstruction(OverMergedTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer)
{
    int childrenCount = node->childrenCount = *LastInDynamicArray(childrenCountersBuffer);
    childrenBuffer->count -= childrenCount;
    
    if (childrenCount > 0)
    {
        int *children = &childrenBuffer->a[childrenBuffer->count];
        
        node->children = malloc(childrenCount * sizeof *children);
        if (!node->children)
            debug("bad node children malloc");
        
        for (int i = 0; i < childrenCount; ++i)
            node->children[i] = children[i];
    }
    
    --(childrenCountersBuffer->count);
}

OverMergedTree *OverMergeTrees(SuffixTree *evenTree, SuffixTree *oddTree, int *s, int n)
{
    OverMergedTree *omt = CreateOverMergedTree();
    
    DynamicArray *childrenCountersBuffer = CreateDynamicArray(1), *childrenBuffer = CreateDynamicArray(1);    
    DfsPosition *px = CreateDfsPosition(evenTree, s, 0, 0), *py = CreateDfsPosition(oddTree, s, 0, 1);
    
    // euler tour structures
    DynamicArray *dfsToNode = CreateDynamicArray(1), *dfsDepths = CreateDynamicArray(1);
    
    int *suffixToDfs = malloc(n * sizeof *suffixToDfs);
    int evenLeftChild, oddLeftChild;
    int depth = 0;
    evenLeftChild = oddLeftChild = 0;
    // ---------------------
        
    // create root node
    int i = AppendNodeToOverMergedTree(omt, 0, px, py);
    PushToDynamicArray(childrenCountersBuffer, 0);
    
    // put root for clarity
    PushToDynamicArray(dfsDepths, depth++);
    PushToDynamicArray(dfsToNode, i);
    // --------------------------------
    
    while (!EndOfDfs(px) || !EndOfDfs(py))
    {
        if (CompareAndSwapDfsPositions(&px, &py))
        {
            // we can copy px branch, but copy only it's root
            i = AppendNodeToOverMergedTree(omt, i, px, NULL);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, i);            
            
            // px children hack
            NextStepOfDfs(px, px->node->depth + 1);
            *LastInDynamicArray(px->lastChild) = px->node->childrenCount;
            NextStepOfDfs(px, 0);
            // now it point to next step after child we've added, i.e. we skipped whole inner branch
            
            // this node is root of subbranch of odd or even tree
            if (omt->nodes[i].evenIndex != -1)
            {
                SetSuffixesToDfs(
                    evenTree, omt->nodes[i].evenIndex, evenTree->suffixArray, 
                    suffixToDfs, dfsDepths->count, &evenLeftChild
                );
            }
            else
            {
                SetSuffixesToDfs(
                    oddTree, omt->nodes[i].oddIndex, oddTree->suffixArray, 
                    suffixToDfs, dfsDepths->count, &oddLeftChild
                );
            }
            PushToDynamicArray(dfsDepths, depth++);
            PushToDynamicArray(dfsToNode, i);
            // -----------------------------------------------------------
        }
        else
        {
            int lx = GetEdgeLength(px), ly = GetEdgeLength(py);
            
            if (lx < ly)
            {
                SwapPositions(&px, &py);
                Swap(&lx, &ly);
            }
                        
            if (lx > ly)
            {
                // insert node in px->tree
                BreakSuffixTreeEdgeByCustomLength(px->tree, px->ind, *LastInDynamicArray(px->lastChild), ly);
            }
            
            //merge edges
            i = AppendNodeToOverMergedTree(omt, i, px, py);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, i);
            
            // take care of inner (for resulting overmerged tree) leaf node
            if (evenTree->nodes[omt->nodes[i].evenIndex].leaf != -1)
                suffixToDfs[evenTree->suffixArray->a[evenLeftChild++]] = dfsDepths->count;
            if (oddTree->nodes[omt->nodes[i].oddIndex].leaf != -1)
                suffixToDfs[oddTree->suffixArray->a[oddLeftChild++]] = dfsDepths->count;
                
            // added node is odd/even ==> don't touch appearance
            PushToDynamicArray(dfsDepths, depth++);
            PushToDynamicArray(dfsToNode, i);
            // -------------------------------------------------
            
            NextStepOfDfs(px, 0);
            NextStepOfDfs(py, 0);
        }
        
        
        while (omt->nodes[i].depth > px->node->depth && omt->nodes[i].depth > py->node->depth)
        {
            CompleteOverMergedTreeNodeConstruction(&(omt->nodes[i]), childrenCountersBuffer, childrenBuffer);
            i = omt->nodes[i].parent;
            
            // go upward
            PushToDynamicArray(dfsDepths, --depth);
        }
    }
    
    if (i != 0)
        debug("воу, воу, палехчи");     
        
    SetSuffixesToDfs(evenTree, -1, evenTree->suffixArray, suffixToDfs, dfsDepths->count, &evenLeftChild);
    SetSuffixesToDfs(oddTree, -1, oddTree->suffixArray, suffixToDfs, dfsDepths->count, &oddLeftChild);
        
    // complete root's children building
    CompleteOverMergedTreeNodeConstruction(&(omt->nodes[0]), childrenCountersBuffer, childrenBuffer);
    omt->oddTree = oddTree;
    omt->evenTree = evenTree;
    
    // complete overmerged tree's euler tour
    omt->eulerTour = CreateOverMergedTreeEulerTour(n, dfsDepths, dfsToNode, suffixToDfs, omt);
    
    return omt;
}

OverMergedTreeEulerTour *CreateOverMergedTreeEulerTour(int n, DynamicArray *dfsDepths, DynamicArray *dfsToNode, int *suffixToDfs, OverMergedTree *tree)
{    
    OverMergedTreeEulerTour *eulerTour = calloc(1, sizeof *eulerTour);
    eulerTour->n = n;
    eulerTour->dfsDepths = dfsDepths;
    eulerTour->dfsToNode = dfsToNode;
    eulerTour->suffixToDfs = suffixToDfs;
    eulerTour->tree = tree;
    
    return eulerTour;
}

void FreeOverMergedTreeEulerTour(OverMergedTreeEulerTour *eulerTour)
{
    if (!eulerTour)
        return;
        
    debug("free EulerTour: started");
    MemFree(eulerTour->suffixToDfs);
    FreeDynamicArray(eulerTour->dfsToNode);
    FreeDynamicArray(eulerTour->dfsDepths);
    MemFree(eulerTour);
    debug("free EulerTour: finished");
}

SuffixTree *BuildSuffixTreeFromOverMergedTree(OverMergedTree *omt, int *s, int n)
{    
    OverMergedTreeEulerTour *eulerTour = omt->eulerTour;
    LcaTable *lcaTable = CreateLcaTable(eulerTour->dfsDepths, eulerTour->dfsToNode);
    
    SuffixTree *oddTree = omt->oddTree, *evenTree = omt->evenTree;
    SuffixTree *st = CreateSuffixTree(0);
    DynamicArray *omtStack = CreateDynamicArray(1), 
        *stStack = CreateDynamicArray(1),
        *childStack = CreateDynamicArray(1);
    int iOmt, iSt, ch;
    
    PushToDynamicArray(omtStack, 0);
    PushToDynamicArray(stStack, 0);
    PushToDynamicArray(childStack, -1);
    
    while (omtStack->count > 0)
    {
        iOmt = PopFromDynamicArray(omtStack);
        iSt = PopFromDynamicArray(stStack);
        ch = PopFromDynamicArray(childStack);
        
        OverMergedTreeNode *omtNode = &(omt->nodes[iOmt]);
        
        if (-1 != omtNode->evenIndex && -1 != omtNode->oddIndex)
        {
            // odd/even node
            int d = (0 != iOmt) 
                ? omt->nodes[GetLcaForOverMergedTree(lcaTable, eulerTour, omtNode->evenIndex + 1, omtNode->oddIndex + 1)].depth + 1 
                : 0;
                
            SuffixTreeNode 
                *evenNode = &(evenTree->nodes[omtNode->evenIndex]), 
                *oddNode = &(oddTree->nodes[omtNode->oddIndex]);       
            
            if (d < evenNode->depth)
            {
                // need to brake edge and append 2 subtree for odd and even branch separately                    
                int j = AppendChildToSuffixTreeNode(st, iSt, ch, evenNode->from, d, -1, 2, NULL);
                    
                int evenFrom = evenNode->from - GetParentDepth(evenTree, omtNode->evenIndex) + d,
                    oddFrom = oddNode->from - GetParentDepth(oddTree, omtNode->oddIndex) + d,
                    evenBranchIndex = s[evenFrom] < s[oddFrom] ? 0 : 1;
                
                AppendSubtreeToSuffixTree(st, j, evenBranchIndex, evenTree, omtNode->evenIndex);
                AppendSubtreeToSuffixTree(st, j, (evenBranchIndex + 1) & 1, oddTree, omtNode->oddIndex);
            }
            else
            {            
                // just append node and go through recursion
                int j = AppendChildToSuffixTreeNode(st, iSt, ch, evenNode->from, evenNode->depth, evenNode->leaf, omtNode->childrenCount, NULL);
                for (int k = 0; k < omtNode->childrenCount; ++k)
                {
                    PushToDynamicArray(omtStack, omtNode->children[k]);
                    PushToDynamicArray(stStack, j);
                    PushToDynamicArray(childStack, k);
                }
            }
        }
        else
        {
            if (-1 != omtNode->evenIndex)
                AppendSubtreeToSuffixTree(st, iSt, ch, evenTree, omtNode->evenIndex);
            else
                AppendSubtreeToSuffixTree(st, iSt, ch, oddTree, omtNode->oddIndex);
        }
    }
    
    st->leavesCount = n;
    
    FreeLcaTable(lcaTable);
    return st;
}

int GetLcaForOverMergedTree(LcaTable *lcaTable, OverMergedTreeEulerTour *eulerTour, int s1, int s2) 
{       
    printf("(%d, %d) ", s1, s2);
    fflush(stdout);
    
	int l = eulerTour->suffixToDfs[s1];
    int r = eulerTour->suffixToDfs[s2];        
	if (l > r)
        Swap(&l, &r);
        
    return GetLca(lcaTable, l, r);
}