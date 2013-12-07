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
    
    MemFree(sa->lcp);
    MemFree(sa->a);
    MemFree(sa->suffixToRank);
    MemFree(sa);
}


/* ---------------------- SUFFIX TREE ------------------------ */
SuffixTree *CreateSuffixTree(int withRoot)
{
    SuffixTree *st = calloc(1, sizeof *st);
    st->count = 0;
    st->capacity = 1;
    st->leavesCount = 0;
    st->nodes = malloc(st->capacity * sizeof *st->nodes);
    
    if (st->nodes)
    {
        // create default root if needed
        if (withRoot)
            AppendChildToSuffixTreeNode(st, 0, -1, 0, 0, -1, 0, NULL);
    }
    else
        debugPrint("bad nodes malloc in CreateSuffixTree\n");
        
    
    return st;
}

void FreeSuffixTree(SuffixTree *st)
{
    if (!st)
        return;

    for (int i = 0; i < st->count; i++)
        MemFree(st->nodes[i].children);
    MemFree(st->nodes);
    
    FreeSuffixArray(st->suffixArray);
    MemFree(st);
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
            debugPrint("bad suffix tree nodes realloc");
    }
    st->nodes[i].children = NULL;
    ++(st->count);
    
    return i;
}

inline int GetParentDepth(SuffixTree *st, int i)
{
    return 0 != i ? st->nodes[st->nodes[i].parent].depth : 0;
}
inline int GetSuffixForNode(SuffixTree *st, int i)
{
    return st->nodes[i].from - GetParentDepth(st, i);
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

int AppendSubtreeToSuffixTree(SuffixTree *st, int parent, int childOrder, SuffixTree *srcTree, int srcSubTreeRoot, int rootFrom)
{
    DynamicArray *childStack = CreateDynamicArray(1);
    SuffixTreeNode *srcNode = &(srcTree->nodes[srcSubTreeRoot]);
    PushToDynamicArray(childStack, childOrder);
    
    int cur = AppendChildToSuffixTreeNode(st, parent, childOrder, rootFrom, srcNode->depth, srcNode->leaf, srcNode->childrenCount, NULL);
    int root = cur;
    PushToDynamicArray(childStack, 0);
    
    while (1)
    {
        if (*LastInDynamicArray(childStack) < st->nodes[cur].childrenCount)
        {
            srcNode = &(srcTree->nodes[srcNode->children[*LastInDynamicArray(childStack)]]);
            int from = srcNode->from;
            
            // skip 'em
            while (srcNode->childrenCount == 1 && srcNode->leaf == -1)
            {
                int child = srcNode->children[0];
                srcTree->nodes[srcNode->parent].children[*LastInDynamicArray(childStack)] = child;
                srcTree->nodes[child].parent = srcNode->parent;
                
                srcNode = &(srcTree->nodes[child]);
            }
                
            cur = AppendChildToSuffixTreeNode(st, cur, *LastInDynamicArray(childStack), from, srcNode->depth, srcNode->leaf, srcNode->childrenCount, NULL);
            PushToDynamicArray(childStack, 0);
        }
        else
        {
            if (st->nodes[cur].parent == parent)
                break;
            
            srcNode = &(srcTree->nodes[srcNode->parent]);
            cur = st->nodes[cur].parent;
            
            PopFromDynamicArray(childStack);
            ++(*LastInDynamicArray(childStack));
        }        
    }
    
    FreeDynamicArray(childStack);
    return root;
}

void BreakSuffixTreeEdgeByCustomLength(SuffixTree *tree, int parent, int childOrder, int edgeLen)
{    
    int oldChild = tree->nodes[parent].children[childOrder];    
    int newNode = AppendChildToSuffixTreeNode(tree, parent, childOrder, tree->nodes[oldChild].from, tree->nodes[parent].depth + edgeLen, -1, 1, NULL);
    
    tree->nodes[oldChild].parent = newNode;
    tree->nodes[oldChild].from += edgeLen;
    tree->nodes[newNode].children[0] = oldChild;
}

void AddChildToBufferAndMakeItCurrent(DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer, int j)
{
    //add child
    ++(*LastInDynamicArray(childrenCountersBuffer));
    PushToDynamicArray(childrenBuffer, j);
    
    //initialize new counter for this child
    PushToDynamicArray(childrenCountersBuffer, 0);
}

/* ----------- internal methods ------------ */
int CreateAndInitializeNextSuffixTreeNode(SuffixTree *st, int parent, int suffixIndex, int depth, int leaf);
void CompleteSuffixTreeNodeConstruction(SuffixTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer);
/* ----------------------------------------- */

int CreateAndInitializeNextSuffixTreeNode(SuffixTree *st, int parent, int suffixIndex, int depth, int leaf)
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
            debugPrint("bad node children malloc");
        
        for (int i = 0; i < childrenCount; ++i)
            node->children[i] = children[i];
    }
    
    --(childrenCountersBuffer->count);
}

int IsSubstring(SuffixTree *st, int *pattern, int *s, int n)
{
    int i = 0;
    SuffixTreeNode *node = &(st->nodes[0]);
    
    while (i < n && node->childrenCount > 0)
    {
        int l = 0, r = node->childrenCount;
        while (l < r - 1)
        {
            int m = (l + r) >> 1;
            if (s[i] < pattern[st->nodes[node->children[m]].from])
                r = m;
            else
                l = m;
        }
        
        node = &(st->nodes[node->children[l]]);
        int j = 0;
        while (i < n && i < node->depth)
        {
            if (s[i] != pattern[node->from + j])
                return 0;
            ++i;
            ++j;
        }
    }
    
    return i == n;
}

/* ---------------------- CONVERTERS ------------------------ */

SuffixTree *CreateSuffixTreeFromSuffixArray(SuffixArray *sa, int strLen)
{
    int n = sa->n;
    int *lcp = sa->lcp, *a = sa->a;
    
    SuffixTree *st = CreateSuffixTree(1);
    DynamicArray *childrenCountersBuffer = CreateDynamicArray(1), *childrenBuffer = CreateDynamicArray(1);
    
    // root
    PushToDynamicArray(childrenCountersBuffer, 0);
    
    // add first leaf node
    int j = CreateAndInitializeNextSuffixTreeNode(st, 0, a[0], strLen - a[0], a[0]);
    AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
        
    int i = 0;   
    while (i < n - 1)
    {
        // we go over suffix array and
        // every step we start in some node with index j and : 
        //  * lcp < current depth ==> goes up until lcp < current depth. 
        //      At the end insert inner node if needed (there's no node with depth = lcp)
        //  * lcp > current depth ==> add new inner node as a child
        // now we stand at node with depth = lcp, so just add leaf node
        
        if (st->nodes[j].depth < lcp[i])
        {
            // add inner node with lcp depth;
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
                // insert node
                
                int childSuffixIndex = st->nodes[child].from - st->nodes[j].depth;
             
                j = CreateAndInitializeNextSuffixTreeNode(st, j, childSuffixIndex, lcp[i], -1);
                
                // fix old parent of @child
                *LastInDynamicArray(childrenBuffer) = j;
                // fix @child
                st->nodes[child].parent = j;
                st->nodes[child].from = childSuffixIndex + lcp[i];
                
                // push @child to j children
                PushToDynamicArray(childrenCountersBuffer, 1);
                PushToDynamicArray(childrenBuffer, child);
            }
        }
        
        // add leaf node depth;
        if (lcp[i] < strLen - a[i + 1])
        {
            j = CreateAndInitializeNextSuffixTreeNode(st, j, a[i + 1], strLen - a[i + 1], a[i + 1]);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
        }
        else
        {
            // inner leaf node
            st->nodes[j].leaf = a[i + 1];
        }
            
        ++i;
    }
    
    // complete remaining nodes
    while (childrenCountersBuffer->count > 0)
    {
        CompleteSuffixTreeNodeConstruction(&st->nodes[j], childrenCountersBuffer, childrenBuffer);
        j = st->nodes[j].parent;
    }
    
    st->suffixArray = sa;
    st->leavesCount = n;
    
    // free resources
    FreeDynamicArray(childrenBuffer);
    FreeDynamicArray(childrenCountersBuffer);
    
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
        if (ch < st->nodes[i].childrenCount)
        {
            PushToDynamicArray(childStack, ch);
            i = st->nodes[i].children[ch];
            ch = 0;        
        }
        else
        {
            if (st->nodes[i].leaf != -1)
            {
                a[j] = st->nodes[i].leaf;
                if (j > 0) 
                    lcp[j - 1] = d;
                ++j;
            }
            
            i = st->nodes[i].parent;
            ch = PopFromDynamicArray(childStack) + 1;
            d = st->nodes[i].depth;
        }
    }
    a[n] = n;
    lcp[n-1] = 0;
    
    // free resources
    FreeDynamicArray(childStack);
    
    return CreateSuffixArray(lcp, a, n);
}

/* --------------------------EULER TOUR FOR SUFFIX ARRAY ----------------------- */

/* ------ internal methods ----------- */
SuffixTreeEulerTour *CreateSuffixTreeEulerTour(int n, DynamicArray *dfsDepths, DynamicArray *dfsToNode, int *rankToDfs, int *suffixToRank, SuffixTree *tree);
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
    if (!eulerTour)
        return;
        
    FreeDynamicArray(eulerTour->dfsToNode);
    FreeDynamicArray(eulerTour->dfsDepths);
    
    MemFree(eulerTour->rankToDfs);
    MemFree(eulerTour);
}

SuffixTreeEulerTour *GetSuffixTreeEulerTour(SuffixTree *st)
{
    int *suffixToRank = st->suffixArray->suffixToRank;
    DynamicArray *childStack = CreateDynamicArray(1);
    int i = 0, ch = 0, j = 0, d = -1, n = st->leavesCount;
        
    int *rankToDfs = malloc(n * sizeof *rankToDfs);
    DynamicArray *dfsDepths = CreateDynamicArray(2 * (n + 1)),
        *dfsToNode = CreateDynamicArray(2 * (n + 1));  
        
    // push root
    PushToDynamicArray(dfsDepths, ++d);
    PushToDynamicArray(dfsToNode, i);              
        
    while (j < n)
    {
        if (ch < st->nodes[i].childrenCount)
        {
            PushToDynamicArray(childStack, ch);
            i = st->nodes[i].children[ch];
            ch = 0;
            
            PushToDynamicArray(dfsDepths, ++d);
            PushToDynamicArray(dfsToNode, i);            
        }
        else
        {
            if (st->nodes[i].leaf != -1)
            {
                // leaf
                rankToDfs[suffixToRank[st->nodes[i].leaf >> 1]] = dfsDepths->count - 1;
                ++j;
            }
            
            i = st->nodes[i].parent;
            ch = PopFromDynamicArray(childStack) + 1;
            
            PushToDynamicArray(dfsDepths, --d);
            PushToDynamicArray(dfsToNode, i);
        }
    }
    
    // free resources
    FreeDynamicArray(childStack);
    
    return CreateSuffixTreeEulerTour(n, dfsDepths, dfsToNode, rankToDfs, suffixToRank, st);
}

inline int GetRankOfSuffix(int *suffixToRank, int n, int i)
{
    return (i >> 1) >= n ? -1 : suffixToRank[i >> 1];
}

int GetLcpForSuffixTree(LcaTable *lcaTable, SuffixTreeEulerTour *eulerTour, int v1, int v2) 
{    
    int nv1 = GetRankOfSuffix(eulerTour->suffixToRank, eulerTour->n, v1), 
        nv2 = GetRankOfSuffix(eulerTour->suffixToRank, eulerTour->n, v2);
                
    if (nv1 < 0 || nv2 < 0)
        return 0;
    
	int l = eulerTour->rankToDfs[nv1];
    int r = eulerTour->rankToDfs[nv2];        
	if (l > r)
        Swap(&l, &r);
        
    return eulerTour->tree->nodes[GetLca(lcaTable, l, r)].depth;
}