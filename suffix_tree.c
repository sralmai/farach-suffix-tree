#include <stdlib.h>
#include <stdio.h>

#include "suffix_tree.h"


/* --------------------- SUFFIX ARRAY ----------------------- */
SuffixArray *CreateSuffixArray(int *lcp, int *a, int n)
{
    SuffixArray *sa = calloc(1, sizeof *sa);
    sa->lcp = lcp;
    sa->a = a;
    sa->n = n;
    
    return sa;
}

void FreeSuffixArray(SuffixArray *sa)
{
    if (!sa)
        return;
    debug("free SuffixArray: started");
    MemFree(sa->lcp);
    MemFree(sa->a);
    MemFree(sa);
    debug("free SuffixArray: finished");
}


/* ---------------------- SUFFIX TREE ------------------------ */
SuffixTree *CreateSuffixTree(int withRoot)
{
    SuffixTree *st = calloc(1, sizeof *st);
    st->count = 0;
    st->capacity = 1;
    st->nodes = malloc(st->capacity * sizeof *st->nodes);
    if (!st->nodes)
        debug("bad nodes malloc in CreateSuffixTree");
        
    if (withRoot)
    {
        AllocateNextNodeIndexInSuffixTree(st);
        st->nodes[0].leaf = -1;
    }
    return st;
}

void FreeSuffixTree(SuffixTree *st)
{
    debug("free SuffixTree: started");
    FreeSuffixArray(st->suffixArray);
    
    for (int i = 0; i < st->count; i++)
        MemFree(st->nodes[i].children);
    MemFree(st->nodes);
    
    MemFree(st);
    debug("free SuffixTree: finished");
}

int AllocateNextNodeIndexInSuffixTree(SuffixTree *st)
{    
    int i = st->count;
    if (i == st->capacity)
    {        
        SuffixTreeNode *t = realloc(st->nodes, 2 * st->capacity * sizeof *st->nodes);
        if (t)
        {
            st->nodes = t;
            st->capacity = st->capacity << 1;
        }
        else
            debug("bad suffix tree nodes realloc");
    }
    st->nodes[i].children = NULL;
    ++(st->count);
    
    return i;
}

inline int GetParentDepth(SuffixTree *st, int i)
{
    return 0 != i ? st->nodes[st->nodes[i].parent].depth : 0;
}

inline int CopyChildToSuffixTree(SuffixTree *st, int parent, int childOrder, SuffixTreeNode *srcNode)
{
    return AppendChildToSuffixTreeNode(st, parent, childOrder, srcNode->from, srcNode->depth, srcNode->leaf, srcNode->childrenCount,  NULL);
}

int AppendChildToSuffixTreeNode(SuffixTree *st, int parent, int childOrder, int from, int depth, int leaf, int childrenCount, int *children)
{
    int i = AllocateNextNodeIndexInSuffixTree(st);
    if (-1 != childOrder)
        st->nodes[parent].children[childOrder] = i;
        
    SuffixTreeNode *node = &(st->nodes[i]);
    node->parent = parent;
    node->from = from;
    node->depth = depth;
    node->leaf = leaf;
    node->childrenCount = childrenCount;
    if (childrenCount > 0)
    {
        node->children = malloc(childrenCount * sizeof *(node->children));
        if (children)
        {
            for (int j = 0; j < childrenCount; j++)
                node->children[j] = children[j];
        }
    }
        
    return i;
}

void AppendSubtreeToSuffixTree(SuffixTree *st, int parent, int childOrder, SuffixTree *tree, int root)
{
    DynamicArray *child = CreateDynamicArray(1);
    SuffixTreeNode *srcNode = &(tree->nodes[root]);
    int cur = parent, c = childOrder;
    PushToDynamicArray(child, childOrder);
    
    int stopFlag = 0;
    while (1)
    {
        while (c >= st->nodes[cur].childrenCount)
        {
            if (srcNode->parent == parent)
            {
                stopFlag = 1;
                break;
            }
            
            srcNode = &(tree->nodes[srcNode->parent]);
            cur = st->nodes[cur].parent;
            
            PopFromDynamicArray(child);
            c = *LastInDynamicArray(child) + 1;
        }
        
        if (stopFlag)
            break;
        
        cur = CopyChildToSuffixTree(st, cur, c, srcNode);
        PushToDynamicArray(child, 0);
        c = 0;
        srcNode = &(tree->nodes[srcNode->children[c]]);
    }
}

void BreakSuffixTreeEdgeByCustomLength(SuffixTree *tree, int parent, int childOrder, int edgeLen)
{
    SuffixTreeNode *parentNode = &(tree->nodes[parent]);    
    int oldChild = parentNode->children[childOrder];
    
    int newNode = AppendChildToSuffixTreeNode(tree, parent, childOrder, tree->nodes[oldChild].from, parentNode->depth + edgeLen, -1, 1, NULL);
    
    tree->nodes[oldChild].parent = newNode;
    tree->nodes[oldChild].from += edgeLen;
    tree->nodes[newNode].children[0] = oldChild;
}

void AddChildToBufferAndMakeItCurrent(DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer, int j)
{
    //add child
    ++(childrenCountersBuffer->count);
    PushToDynamicArray(childrenBuffer, j);
    
    //initialize new counter for this child
    PushToDynamicArray(childrenCountersBuffer, 0);
}

/* ----------- internal methods ------------ */
int CreateAndInitializeNextSuffixTreeNode(SuffixTree *st, int parent, int suffixIndex, int depth, int leaf);
void CompleteSuffixTreeNodeConstruction(SuffixTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer);
/* ----------------------------------------- */

inline int CreateAndInitializeNextSuffixTreeNode(SuffixTree *st, int parent, int suffixIndex, int depth, int leaf)
{
    int i = AllocateNextNodeIndexInSuffixTree(st);
    SuffixTreeNode *node = &(st->nodes[i]);
    
    node->parent = parent;
    node->depth = depth;
    node->from = suffixIndex + st->nodes[parent].depth;
    node->leaf = leaf;
    
    return i;
}

void CompleteSuffixTreeNodeConstruction(SuffixTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer)
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


/* ---------------------- CONVERTERS ------------------------ */

SuffixTree *CreateSuffixTreeFromSuffixArray(SuffixArray *sa)
{
    int n = sa->n;
    int *lcp = sa->lcp, *a = sa->a;
    
    SuffixTree *st = CreateSuffixTree(1);
    DynamicArray *childrenCountersBuffer = CreateDynamicArray(1), *childrenBuffer = CreateDynamicArray(1);
    
    // root
    PushToDynamicArray(childrenCountersBuffer, 0);
    
    //add first leaf node
    int j = CreateAndInitializeNextSuffixTreeNode(st, 0, a[0], n - a[0], a[0]);
    AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
    
    int i = 0;   
    while (i < n)
    {
        // we go over suffix array and
        // every step we start in some node with index j and : 
        //  * lcp < current depth ==> goes up until lcp < current depth. 
        //      At the end insert inner node if needed (there's no node with depth = lcp)
        //  * lcp > current depth ==> add new inner node as a child
        // now we stand at node with depth = lcp, so just add leaf node
        
        if (st->nodes[j].depth < lcp[i])
        {
            //add inner node with lcp depth;
            j = CreateAndInitializeNextSuffixTreeNode(st, j, a[i + 1], lcp[i], -1);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
        }
        else if (st->nodes[j].depth > lcp[i])
        {
            int child;
            do
            {
                CompleteSuffixTreeNodeConstruction(&st->nodes[j], childrenCountersBuffer, childrenBuffer);
                child = j;
                j = st->nodes[j].parent;
            }
            while (st->nodes[j].depth > lcp[i]);
            
            if (st->nodes[j].depth < lcp[i])
            {
                //insert node;
                j = CreateAndInitializeNextSuffixTreeNode(st, j, a[i + 1], lcp[i], -1);
                st->nodes[child].parent = j;
                *LastInDynamicArray(childrenBuffer) = j;
                AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
            }
        }
        
        //add leaf node depth;
        if (lcp[i] < n - a[i + 1])
        {
            j = CreateAndInitializeNextSuffixTreeNode(st, j, a[i + 1], n - a[i + 1], a[i + 1]);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
        }
        else
            st->nodes[j].leaf = a[i + 1];
    }
    
    //complete remaining nodes
    while (childrenCountersBuffer->count > 0)
    {
        CompleteSuffixTreeNodeConstruction(&st->nodes[j], childrenCountersBuffer, childrenBuffer);
        j = st->nodes[j].parent;
    }
    
    st->suffixArray = sa;
    
    return st;
}

SuffixArray *CreateSuffixArrayFromSuffixTree(SuffixTree *st)
{
    DynamicArray *childStack = CreateDynamicArray(1);
    int i = 0, ch = 0, j = 0, d = -1, n = st->leavesCount;
    int *a = malloc((n + 1) * sizeof *a),
        *lcp = malloc(n * sizeof *lcp);
        
    while (j < n)
    {
        while (ch >= st->nodes[i].childrenCount)
        {
            i = st->nodes[i].parent;
            ch = PopFromDynamicArray(childStack) + 1;
            d = st->nodes[i].depth;
        }
        
        while (ch != 0 || st->nodes[i].leaf != 1)
        {
            PushToDynamicArray(childStack, ch);
            i = st->nodes[i].children[ch];
            ch = 0;
        }
        
        a[j] = st->nodes[i].from - st->nodes[i].depth;
        if (d != -1)
        {
            // first leaf won't count
            lcp[j] = d;
        }
        ++j;
        d = st->nodes[i].depth;
    }
    a[n] = n;
    lcp[n-1] = 0;
    
    return CreateSuffixArray(lcp, a, n);
}

/* --------------------------EULER TOUR FOR SUFFIX ARRAY ----------------------- */

/* ------ internal methods ----------- */
SuffixTreeEulerTour *CreateSuffixTreeEulerTour(
    int n, DynamicArray *dfsDepths, DynamicArray *dfsToNode, int *rankToDfs, int *suffixToRank, SuffixTree *tree
);
int GetRankOfSuffix(int *suffixToRank, int n, int i);
/* ----------------------------------- */

SuffixTreeEulerTour *CreateSuffixTreeEulerTour(int n, DynamicArray *dfsDepths, DynamicArray *dfsToNode, int *rankToDfs, int *suffixToRank, SuffixTree *tree)
{    
    SuffixTreeEulerTour *eulerTour = calloc(1, sizeof *eulerTour);
    eulerTour->n = n;
    eulerTour->dfsDepths = dfsDepths;
    eulerTour->dfsToNode = dfsToNode;
    eulerTour->rankToDfs = rankToDfs;
    eulerTour->suffixToRank = suffixToRank;
    eulerTour->tree = tree;
    
    return eulerTour;
}

void FreeSuffixTreeEulerTour(SuffixTreeEulerTour *eulerTour)
{
    debug("free EulerTour: started");
    MemFree(eulerTour->rankToDfs);
    FreeDynamicArray(eulerTour->dfsToNode);
    FreeDynamicArray(eulerTour->dfsDepths);
    MemFree(eulerTour->suffixToRank);
    MemFree(eulerTour);
    debug("free EulerTour: finished");
}

SuffixTreeEulerTour *GetSuffixTreeEulerTour(SuffixTree *st, int *suffixToRank)
{
    debug("create EulerTour for SuffixTree: started");
     
    DynamicArray *childStack = CreateDynamicArray(1);
    int i = 0, ch = 0, j = 0, d = -1, n = st->leavesCount;
        
    int *rankToDfs = malloc(n * sizeof *rankToDfs);
    DynamicArray *dfsDepths = CreateDynamicArray(2 * (n + 1)),
        *dfsToNode = CreateDynamicArray(2 * (n + 1));                
        
    while (j < n)
    {
        while (ch >= st->nodes[i].childrenCount)
        {
            i = st->nodes[i].parent;
            ch = PopFromDynamicArray(childStack) + 1;
            
            PushToDynamicArray(dfsDepths, --d);
            PushToDynamicArray(dfsToNode, i);
        }
        
        while (st->nodes[i].leaf != 1)
        {
            PushToDynamicArray(childStack, ch);
            i = st->nodes[i].children[ch];
            ch = 0;
            
            PushToDynamicArray(dfsDepths, ++d);
            PushToDynamicArray(dfsToNode, i);
        }
        
        int rankOfSuffix = GetRankOfSuffix(suffixToRank, n, st->nodes[i].from - st->nodes[i].depth);
        rankToDfs[rankOfSuffix] = dfsDepths->count - 1;
        ++j;
    }
        
    debugArr(dfsDepths->a, dfsDepths->count);
    debugArr(rankToDfs, n);
    
    debug("create EulerTour for SuffixTree: finished");
    
    return CreateSuffixTreeEulerTour(n, dfsDepths, dfsToNode, rankToDfs, suffixToRank, st);
}

int GetRankOfSuffix(int *suffixToRank, int n, int i)
{
    i = i >> 1;
    if (i >= n)
        return - 1;
    return suffixToRank[i];
}

int GetLcpForSuffixTree(LcaTable *lcaTable, SuffixTreeEulerTour *eulerTour, int v1, int v2) 
{       
    printf("(%d, %d) ", v1, v2);
    fflush(stdout);
    
    int nv1 = GetRankOfSuffix(eulerTour->suffixToRank, eulerTour->n, v1), 
        nv2 = GetRankOfSuffix(eulerTour->suffixToRank, eulerTour->n, v2);
            
    printf("(%d, %d) ", nv1, nv2);
    fflush(stdout);
    
    if (nv1 < 0 || nv2 < 0)
        return 0;
    
	int l = eulerTour->rankToDfs[nv1];
    int r = eulerTour->rankToDfs[nv2];        
	if (l > r)
        Swap(&l, &r);
        
    return eulerTour->tree->nodes[GetLca(lcaTable, l, r)].depth;
}