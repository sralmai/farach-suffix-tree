#include <stdlib.h>
#include <stdio.h>
#include "overmerged_tree.h"


/* --------- internal methods ---------- */
OverMergedTree *CreateOverMergedTree(int capacity);
/* ------------------------------------- */

OverMergedTree *CreateOverMergedTree(int capacity)
{
    OverMergedTree *omt = calloc(1, sizeof *omt);
    omt->count = 0;
    omt->capacity = capacity;
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
    
    node->linkToResultNode = -1;
    node->realDepth = -1;
    
    return nodeIndex;
}

inline int AppendNodeToOverMergedTree(OverMergedTree *omt, int parent, DfsPosition *px, DfsPosition *py)
{
    if (0 == px->treeType)
        return AppendCustomNodeToOverMergedTree(omt, parent, px->tree->nodes[GetChildIndex(px)].depth, GetChildIndex(px), py ? GetChildIndex(py) : -1);
    else
        return AppendCustomNodeToOverMergedTree(omt, parent, px->tree->nodes[GetChildIndex(px)].depth, py ? GetChildIndex(py) : -1, GetChildIndex(px));
}

void SetSuffixesToDfsForSubtree(SuffixTree *st, int parent, int childIndex, int *suffixToDfs, int dfsIndex)
{
    int cur = st->nodes[parent].children[childIndex];
    int leftSuffixRank = st->suffixArray->suffixToRank[(st->nodes[cur].from - st->nodes[parent].depth) >> 1];
    
    while (st->nodes[cur].leaf == -1)
        cur = st->nodes[cur].children[st->nodes[cur].childrenCount - 1];
        
    int rightSuffixRank = st->suffixArray->suffixToRank[st->nodes[cur].leaf >> 1];
    
    while (leftSuffixRank <= rightSuffixRank)
        suffixToDfs[st->suffixArray->a[leftSuffixRank++]] = dfsIndex;
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
    OverMergedTree *omt = CreateOverMergedTree(2 * n);
    
    DynamicArray *childrenCountersBuffer = CreateDynamicArray(1), *childrenBuffer = CreateDynamicArray(1);    
    DfsPosition *px = CreateDfsPosition(evenTree, s, n, 0, 0), *py = CreateDfsPosition(oddTree, s, n, 0, 1);
    
    // euler tour structures
    DynamicArray *dfsToNode = CreateDynamicArray(1), *dfsDepths = CreateDynamicArray(1);    
    int *suffixToDfs = malloc(n * sizeof *suffixToDfs);
    int depth = 0;
    // ---------------------
        
    // create parent node
    int i = AppendCustomNodeToOverMergedTree(omt, 0, 0, 0, 0);
    PushToDynamicArray(childrenCountersBuffer, 0);
    
    // put parent for clarity
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
            
            PushToDynamicArray(dfsDepths, ++depth);
            PushToDynamicArray(dfsToNode, i);
            // -----------------------------------------------------------
            
            // this node is parent of subbranch of odd or even tree
            SetSuffixesToDfsForSubtree(px->tree, px->ind, *LastInDynamicArray(px->lastChild), suffixToDfs, dfsDepths->count - 1);
            
            // px children hack
            NextStepOfDfs(px, px->tree->nodes[px->ind].depth + 1);
            *LastInDynamicArray(px->lastChild) = px->tree->nodes[px->ind].childrenCount;
            NextStepOfDfs(px, 0);
            // now it point to next step after child we've added, i.e. we skipped whole inner branch            
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
            omt->nodes[i].linkToResultNode = -2;
            
            // added node is odd/even ==> don't touch appearance
            PushToDynamicArray(dfsDepths, ++depth);
            PushToDynamicArray(dfsToNode, i);
            // -------------------------------------------------
            
            // take care of inner (for resulting overmerged tree) leaf node
            int pyLeaf = py->tree->nodes[GetChildIndex(py)].leaf;
            if (pyLeaf != -1)
                suffixToDfs[py->tree->suffixArray->a[py->tree->suffixArray->suffixToRank[pyLeaf >> 1]]] = dfsDepths->count - 1;
            else
            {
                int pxLeaf = px->tree->nodes[GetChildIndex(px)].leaf;
                if (pxLeaf != -1)
                    suffixToDfs[px->tree->suffixArray->a[px->tree->suffixArray->suffixToRank[pxLeaf >> 1]]] = dfsDepths->count - 1;
            }
            
            NextStepOfDfs(px, 0);
            NextStepOfDfs(py, 0);
        }
        
        
        while (omt->nodes[i].depth > px->tree->nodes[px->ind].depth && omt->nodes[i].depth > py->tree->nodes[py->ind].depth)
        {
            CompleteOverMergedTreeNodeConstruction(&(omt->nodes[i]), childrenCountersBuffer, childrenBuffer);
            
            if (omt->nodes[i].oddIndex != -1 && omt->nodes[i].evenIndex != -1)
                SetSuffixesForLcaProblem(omt, i, evenTree, oddTree);            
            
            i = omt->nodes[i].parent;
            
            // go upward
            PushToDynamicArray(dfsDepths, --depth);
            PushToDynamicArray(dfsToNode, i);
        }
    }
        
    // complete parent's children building
    CompleteOverMergedTreeNodeConstruction(&(omt->nodes[0]), childrenCountersBuffer, childrenBuffer);
            
    // free resources
    FreeDynamicArray(childrenCountersBuffer);
    FreeDynamicArray(childrenBuffer);
    FreeDfsPosition(px);
    FreeDfsPosition(py);
    
    // complete overmerged tree's euler tour and build 
    BuildLcpTreeOnOverMergedTree(omt, CreateOverMergedTreeEulerTour(n, dfsDepths, dfsToNode, suffixToDfs, omt), evenTree, oddTree);
    
    return omt;
}

void SetSuffixesForLcaProblem(OverMergedTree *omt, int ind, SuffixTree *evenTree, SuffixTree *oddTree)
{
    OverMergedTreeNode *node = &omt->nodes[ind];
    node->oddSuffix = -1;
    node->evenSuffix = -1;

    if (evenTree->nodes[node->evenIndex].leaf != -1)
    {
        node->evenSuffix = evenTree->nodes[node->evenIndex].leaf;
        node->oddSuffix = GetSuffixForNode(oddTree, node->oddIndex);
    }
    else if (oddTree->nodes[node->oddIndex].leaf != -1)
    {
        node->oddSuffix = oddTree->nodes[node->oddIndex].leaf;
        node->evenSuffix = GetSuffixForNode(evenTree, node->evenIndex);
    }
    else
    {
        for (int i = 0; i < node->childrenCount && node->oddSuffix == -1; ++i)
        {
            if (omt->nodes[node->children[i]].evenIndex == -1)
                continue;
                
            node->evenSuffix = GetSuffixForNode(evenTree, omt->nodes[node->children[i]].evenIndex);
            
            for (int j = 0; j < node->childrenCount && node->oddSuffix == -1; ++j)
            {
                if (i == j || omt->nodes[node->children[j]].oddIndex == -1)
                    continue;
                
                node->oddSuffix = GetSuffixForNode(oddTree, omt->nodes[node->children[j]].oddIndex);
            }
        }
    }
    
    if (node->oddSuffix == -1 || node->evenSuffix == -1)
    {
        debugPrint("Life is pain\n");
        return;
    }
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

void BuildLcpTreeOnOverMergedTree(OverMergedTree *omt, OverMergedTreeEulerTour *eulerTour, SuffixTree *evenTree, SuffixTree *oddTree)
{
    LcaTable *lcaTable = CreateLcaTable(eulerTour->dfsDepths, eulerTour->dfsToNode);
    DynamicArray *childStack = CreateDynamicArray(1);
    int cur = 0;
    PushToDynamicArray(childStack, 0);
    omt->nodes[0].realDepth = 0;
    
    while (cur != 0 || *LastInDynamicArray(childStack) < omt->nodes[cur].childrenCount)
    {
        if (*LastInDynamicArray(childStack) < omt->nodes[cur].childrenCount)
        {
            cur = omt->nodes[cur].children[*LastInDynamicArray(childStack)];            
            if (omt->nodes[cur].evenIndex != -1 && omt->nodes[cur].oddIndex != -1)
            {
                int i = 0;        
                while (omt->nodes[cur].realDepth == -1)
                {                
                    int linkedNodeIndex = GetLcaForOverMergedTree(lcaTable, eulerTour, omt->nodes[cur].evenSuffix + 1, omt->nodes[cur].oddSuffix + 1);
                    
                    PushToDynamicArray(childStack, cur);
                    cur = linkedNodeIndex;
                    ++i;
                }
                
                while (i > 0)
                {
                    omt->nodes[*LastInDynamicArray(childStack)].realDepth = omt->nodes[cur].realDepth + 1;
                    cur = PopFromDynamicArray(childStack);
                    --i;
                }
                
                PushToDynamicArray(childStack, 0);
            }
            else
            {
                // go upward
                cur = omt->nodes[cur].parent;
                ++(*LastInDynamicArray(childStack));
            }
        }
        else
        {
            // go upward
            cur = omt->nodes[cur].parent;
            PopFromDynamicArray(childStack);
            ++(*LastInDynamicArray(childStack));
        }
    }
    
    FreeDynamicArray(childStack);
    FreeLcaTable(lcaTable);
    FreeOverMergedTreeEulerTour(eulerTour);
}

SuffixTree *BuildSuffixTreeFromOverMergedTree(OverMergedTree *omt, SuffixTree *evenTree, SuffixTree *oddTree, int *s, int n)
{      
    SuffixTree *st = CreateSuffixTree(2 * n, 0);
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
                *evenNode = &(evenTree->nodes[omtNode->evenIndex]),
                *oddNode = &(oddTree->nodes[omtNode->oddIndex]);
            
            if (omtNode->realDepth < evenNode->depth)
            {
                // need to brake edge and append 2 subtree for odd and even branch separately                    
                iSt = AppendChildToSuffixTreeNode(st, iSt, ch, evenNode->from, omtNode->realDepth, -1, 2, NULL);
                omtNode->linkToResultNode = iSt;
                    
                int evenFrom = GetSuffixForNode(evenTree, omtNode->evenIndex) + omtNode->realDepth,
                    oddFrom = GetSuffixForNode(oddTree, omtNode->oddIndex) + omtNode->realDepth,
                    evenBranchIndex = s[evenFrom] < s[oddFrom] ? 0 : 1;
                
                // append even subtree parent and then recursive append all it's subtree
                AppendSubtreeToSuffixTree(st, iSt, evenBranchIndex, evenTree, omtNode->evenIndex, evenFrom);
                
                // append odd subtree parent and then recursive append all it's subtree
                AppendSubtreeToSuffixTree(st, iSt, (evenBranchIndex + 1) & 1, oddTree, omtNode->oddIndex, oddFrom);
            }
            else
            {                    
                // just append node and go through recursion                
                int from = evenNode->from < oddNode->from ? evenNode->from : oddNode->from, 
                    depth = evenNode->from < oddNode->from ? evenNode->depth : oddNode->depth,
                    leaf = evenNode->leaf != -1 ? evenNode->leaf : oddNode->leaf;
                    
                iSt = AppendChildToSuffixTreeNode(st, iSt, ch, from, depth, leaf, omtNode->childrenCount, NULL);
                omtNode->linkToResultNode = iSt;
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
                omtNode->linkToResultNode = AppendSubtreeToSuffixTree(st, iSt, ch, evenTree, omtNode->evenIndex, evenTree->nodes[omtNode->evenIndex].from);
            else
                omtNode->linkToResultNode = AppendSubtreeToSuffixTree(st, iSt, ch, oddTree, omtNode->oddIndex, oddTree->nodes[omtNode->oddIndex].from);
        }
    }
    
    st->leavesCount = n;
    
    // free resources 
    FreeDynamicQueue(omtQueue);
    FreeDynamicQueue(stQueue);
    FreeDynamicQueue(childQueue);
    
    return st;
}

int GetLcaForOverMergedTree(LcaTable *lcaTable, OverMergedTreeEulerTour *eulerTour, int s1, int s2) 
{
    if (s1 >= eulerTour->n || s2 >= eulerTour->n)
        return 0;
        
	int l = eulerTour->suffixToDfs[s1];
    int r = eulerTour->suffixToDfs[s2];        
	if (l > r)
    {
        Swap(&l, &r);
    }
        
    return GetLca(lcaTable, l, r);
}
