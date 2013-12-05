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
        debugPrint("bad nodes malloc in OverMergeTrees");
                
    return omt;
}

void FreeOverMergedTree(OverMergedTree *omt)
{
    if (!omt)
        return;
        
    for (int i = 0; i < omt->count; i++)
        MemFree(omt->nodes[i].children);
    MemFree(omt->nodes);
    
    FreeOverMergedTreeEulerTour(omt->eulerTour);
    MemFree(omt);
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
            debugPrint("bad overmerged tree nodes realloc");
    }
    omt->nodes[i].children = NULL;
    ++omt->count;
    
    return i;
}

int AppendCustomNodeToOverMergedTree(OverMergedTree *omt, int parent, int depth, int evenIndex, int oddIndex)
{     
    int nodeIndex = AllocateNextNodeIndexInOverMergedTree(omt);
    OverMergedTreeNode *node = &(omt->nodes[nodeIndex]);
    
    node->parent = parent;
    node->depth = depth;
    node->evenIndex = evenIndex;
    node->oddIndex = oddIndex;
    
    return nodeIndex;
}

inline int AppendNodeToOverMergedTree(OverMergedTree *omt, int parent, DfsPosition *px, DfsPosition *py)
{
    if (0 == px->treeType)
        return AppendCustomNodeToOverMergedTree(omt, parent, px->tree->nodes[GetChildIndex(px)].depth, GetChildIndex(px), py ? GetChildIndex(py) : -1);
    else
        return AppendCustomNodeToOverMergedTree(omt, parent, px->tree->nodes[GetChildIndex(px)].depth, py ? GetChildIndex(py) : -1, GetChildIndex(px));
}

void SetSuffixesToDfsByPosition(DfsPosition *p, int *suffixToDfs, int dfsIndex)
{
    SetSuffixesToDfs(p->tree, p->ind, *LastInDynamicArray(p->lastChild), p->tree->suffixArray, suffixToDfs, dfsIndex, &(p->lastDfsLeaf));
}
void SetSuffixesToDfs(SuffixTree *st, int parent, int childIndex, SuffixArray *sa, int *suffixToDfs, int dfsIndex, int *lastDfsLeaf)
{
    int rightSuffix = childIndex >= st->nodes[parent].childrenCount ? sa->n : st->nodes[st->nodes[parent].children[childIndex]].from - st->nodes[parent].depth;
    suffixToDfs[rightSuffix] = dfsIndex;
    int prevDfsIndex = suffixToDfs[sa->a[*lastDfsLeaf]];
    
    while (*lastDfsLeaf < sa->n && sa->a[*lastDfsLeaf] != rightSuffix)
    {
        suffixToDfs[sa->a[*lastDfsLeaf]] = prevDfsIndex;
        ++(*lastDfsLeaf);
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
        if (node->children)
        {
            for (int i = 0; i < childrenCount; ++i)
                node->children[i] = children[i];
        }
        else
            debugPrint("bad overmerged node children malloc \n");
        
    }
    
    PopFromDynamicArray(childrenCountersBuffer);
}

OverMergedTree *OverMergeTrees(SuffixTree *evenTree, SuffixTree *oddTree, int *s, int n)
{
    OverMergedTree *omt = CreateOverMergedTree();
    
    DynamicArray *childrenCountersBuffer = CreateDynamicArray(1), *childrenBuffer = CreateDynamicArray(1);    
    DfsPosition *px = CreateDfsPosition(evenTree, s, 0, 0), *py = CreateDfsPosition(oddTree, s, 0, 1);
    
    // euler tour structures
    DynamicArray *dfsToNode = CreateDynamicArray(1), *dfsDepths = CreateDynamicArray(1);    
    int *suffixToDfs = malloc(n * sizeof *suffixToDfs);
    int depth = 0;
    // ---------------------
        
    // create root node
    int i = AppendCustomNodeToOverMergedTree(omt, 0, 0, 0, 0);
    PushToDynamicArray(childrenCountersBuffer, 0);
    
    // put root for clarity
    PushToDynamicArray(dfsDepths, depth);
    PushToDynamicArray(dfsToNode, i);
    // --------------------------------
    
    while (!EndOfDfs(px) || !EndOfDfs(py))
    {
        if (CompareAndSwapDfsPositions(&px, &py))
        {
            // we can copy px branch, but copy only it's root
            i = AppendNodeToOverMergedTree(omt, i, px, NULL);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, i);            
            
            // this node is root of subbranch of odd or even tree
            SetSuffixesToDfsByPosition(px, suffixToDfs, dfsDepths->count);
            
            // px children hack
            NextStepOfDfs(px, px->tree->nodes[px->ind].depth + 1);
            *LastInDynamicArray(px->lastChild) = px->tree->nodes[px->ind].childrenCount;
            NextStepOfDfs(px, 0);
            // now it point to next step after child we've added, i.e. we skipped whole inner branch
            
            PushToDynamicArray(dfsDepths, ++depth);
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
                // insert node in px->tree by breaking current downward edge
                BreakSuffixTreeEdgeByCustomLength(px->tree, px->ind, *LastInDynamicArray(px->lastChild), ly);
            }
            
            //merge edges
            i = AppendNodeToOverMergedTree(omt, i, px, py);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, i);
            
            // take care of inner (for resulting overmerged tree) leaf node
            int pxOnLeaf = px->tree->nodes[px->tree->nodes[px->ind].children[*LastInDynamicArray(px->lastChild)]].leaf != -1;
            SetSuffixesToDfsByPosition(pxOnLeaf ? px : py, suffixToDfs, dfsDepths->count);
                
            // added node is odd/even ==> don't touch appearance
            PushToDynamicArray(dfsDepths, ++depth);
            PushToDynamicArray(dfsToNode, i);
            // -------------------------------------------------
            
            NextStepOfDfs(px, 0);
            NextStepOfDfs(py, 0);
        }
        
        
        while (omt->nodes[i].depth > px->tree->nodes[px->ind].depth && omt->nodes[i].depth > py->tree->nodes[py->ind].depth)
        {
            CompleteOverMergedTreeNodeConstruction(&(omt->nodes[i]), childrenCountersBuffer, childrenBuffer);
            i = omt->nodes[i].parent;
            
            // go upward
            PushToDynamicArray(dfsDepths, --depth);
            PushToDynamicArray(dfsToNode, i);
        }
    }       
        
    SetSuffixesToDfsByPosition(px, suffixToDfs, dfsDepths->count - 1);
    SetSuffixesToDfsByPosition(py, suffixToDfs, dfsDepths->count - 1);
        
    // complete root's children building
    CompleteOverMergedTreeNodeConstruction(&(omt->nodes[0]), childrenCountersBuffer, childrenBuffer);
    
    // complete overmerged tree's euler tour
    omt->eulerTour = CreateOverMergedTreeEulerTour(n, dfsDepths, dfsToNode, suffixToDfs, omt);
    
    // free resources
    FreeDynamicArray(childrenCountersBuffer);
    FreeDynamicArray(childrenBuffer);
    FreeDfsPosition(px);
    FreeDfsPosition(py);
    
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
        
    FreeDynamicArray(eulerTour->dfsToNode);
    FreeDynamicArray(eulerTour->dfsDepths);
    MemFree(eulerTour->suffixToDfs);
    MemFree(eulerTour);
}

SuffixTree *BuildSuffixTreeFromOverMergedTree(OverMergedTree *omt, SuffixTree *evenTree, SuffixTree *oddTree, int *s, int n)
{    
    OverMergedTreeEulerTour *eulerTour = omt->eulerTour;
    LcaTable *lcaTable = CreateLcaTable(eulerTour->dfsDepths, eulerTour->dfsToNode);
    
    SuffixTree *st = CreateSuffixTree(0);
    DynamicQueue *omtQueue = CreateDynamicQueue(1), 
        *stQueue = CreateDynamicQueue(1),
        *childQueue = CreateDynamicQueue(1);
    int iOmt, iSt, ch;
    
    PushToDynamicQueue(omtQueue, 0);
    PushToDynamicQueue(stQueue, 0);
    PushToDynamicQueue(childQueue, -1);
    
    while (omtQueue->count > 0)
    {
        iOmt = PopFromDynamicQueue(omtQueue);
        iSt = PopFromDynamicQueue(stQueue);
        ch = PopFromDynamicQueue(childQueue);
        
        OverMergedTreeNode *omtNode = &(omt->nodes[iOmt]);
        
        if (-1 != omtNode->evenIndex && -1 != omtNode->oddIndex)
        {
            SuffixTreeNode 
                *evenNode = &(evenTree->nodes[omtNode->evenIndex]);
                
            // odd/even node
            int d;
            if (0 != iOmt)
            {
                int linkedNodeIndex = GetLcaForOverMergedTree(lcaTable, eulerTour, GetSuffixForNode(evenTree, omtNode->evenIndex) + 1, GetSuffixForNode(oddTree, omtNode->oddIndex) + 1);
                d = omt->nodes[linkedNodeIndex].depth + 1;
            }
            else
                d = 0;
            
            if (d < evenNode->depth)
            {
                // need to brake edge and append 2 subtree for odd and even branch separately                    
                iSt = AppendChildToSuffixTreeNode(st, iSt, ch, evenNode->from, d, -1, 2, NULL);
                    
                int evenFrom = GetSuffixForNode(evenTree, omtNode->evenIndex) + d,
                    oddFrom = GetSuffixForNode(oddTree, omtNode->oddIndex) + d,
                    evenBranchIndex = s[evenFrom] < s[oddFrom] ? 0 : 1;
                
                // append even subtree root and then recursive append all it's subtree
                AppendSubtreeToSuffixTree(st, iSt, evenBranchIndex, evenTree, omtNode->evenIndex, evenFrom);
                
                // append odd subtree root and then recursive append all it's subtree
                AppendSubtreeToSuffixTree(st, iSt, (evenBranchIndex + 1) & 1, oddTree, omtNode->oddIndex, oddFrom);
            }
            else
            {            
                // just append node and go through recursion
                iSt = AppendChildToSuffixTreeNode(st, iSt, ch, evenNode->from, evenNode->depth, evenNode->leaf, omtNode->childrenCount, NULL);
                for (int k = 0; k < omtNode->childrenCount; ++k)
                {
                    PushToDynamicQueue(omtQueue, omtNode->children[k]);
                    PushToDynamicQueue(stQueue, iSt);
                    PushToDynamicQueue(childQueue, k);
                }
            }
        }
        else
        {
            if (-1 != omtNode->evenIndex)
                AppendSubtreeToSuffixTree(st, iSt, ch, evenTree, omtNode->evenIndex, evenTree->nodes[omtNode->evenIndex].from);
            else
                AppendSubtreeToSuffixTree(st, iSt, ch, oddTree, omtNode->oddIndex, oddTree->nodes[omtNode->oddIndex].from);
        }
    }
    
    st->leavesCount = n;
    
    // free resources
    FreeLcaTable(lcaTable);    
    FreeDynamicQueue(omtQueue);
    FreeDynamicQueue(stQueue);
    FreeDynamicQueue(childQueue);
    
    return st;
}

int GetLcaForOverMergedTree(LcaTable *lcaTable, OverMergedTreeEulerTour *eulerTour, int s1, int s2) 
{
	int l = eulerTour->suffixToDfs[s1];
    int r = eulerTour->suffixToDfs[s2];        
	if (l > r)
    {
        Swap(&l, &r);
    }
        
    return GetLca(lcaTable, l, r);
}