#include <stdlib.h>
#include "suffix_tree_dfs.h"
#include "helpers.h"

DfsPosition *CreateDfsPosition(SuffixTree *st, int *s, int ind, int treeType)
{
    DfsPosition *p = calloc(1, sizeof *p);
    p->tree = st;
    p->treeType = treeType;
    p->s = s;
    p->ind = ind;
    p->node = &(p->tree->nodes[ind]);
    
    p->lastChild = CreateDynamicArray(1);
    PushToDynamicArray(p->lastChild, 0);
    
    return p;
}

void FreeDfsPosition(DfsPosition *p)
{
    FreeDynamicArray(p->lastChild);
    MemFree(p);
}

/* ------------ short inline helpers ------------ */
inline int EndOfDfs(DfsPosition *p)
{
    return 0 == p->node->depth && *LastInDynamicArray(p->lastChild) == p->node->childrenCount;
}

inline int GetEdgeLength(DfsPosition *p)
{
    return p->tree->nodes[*LastInDynamicArray(p->lastChild)].depth - p->node->depth;
}

inline int GetFirstCharOfChildEdge(DfsPosition *p)
{
    return (*LastInDynamicArray(p->lastChild) < p->node->childrenCount) 
        ? p->s[p->tree->nodes[GetChildIndex(p)].from]
        : 0;
}

inline int GetChildIndex(DfsPosition *p)
{
    return p->node->children[*LastInDynamicArray(p->lastChild)];
}
/* --------------------------------------------- */

int CompareAndSwapDfsPositions(DfsPosition **ppx, DfsPosition **ppy)
{
    DfsPosition *px = *ppx, *py = *ppy;
    
    if (px->node->depth == py->node->depth)
    {
        int cx = GetFirstCharOfChildEdge(px);
        int cy = GetFirstCharOfChildEdge(py);
        
        if (cx < cy)
            Swap(ppx, ppy);
        return cx != cy;
    }
    else 
    {
        if (px->node->depth < py->node->depth)
            Swap(ppx, ppy);            
        return 1;
    }
}

int NextStepOfDfs(DfsPosition *p, int minDepth)
{
    if (*LastInDynamicArray(p->lastChild) < p->node->childrenCount)
    {
        // move down through next child edge
        p->ind = GetChildIndex(p);
        p->node = &(p->tree->nodes[p->ind]);
        PushToDynamicArray(p->lastChild, 0);
    }    
    // go upward until we can go downward in next step
    while (*LastInDynamicArray(p->lastChild) >= p->node->childrenCount)
    {
        if (p->node->depth <= minDepth)
            return 0;
        
        // up to parent
        p->ind = p->node->parent;
        p->node = &(p->tree->nodes[p->ind]);
        --(p->lastChild->count);
        ++(*LastInDynamicArray(p->lastChild));
    }    
    return 1;    
}

// int AppendNodeToSuffixTree(SuffixTree *st, int parent, DfsPosition *p, int copyChildren)
// {
    // int i = AllocateNextNodeIndexInSuffixTree(st);
    // st->nodes[parent].children[*LastInDynamicArray(p->lastChild)] = i;
    // SuffixTreeNode *node = &(st->nodes[i]);
    
    // node->parent = parent;
    // node->depth = p->node->depth;
    // node->from = p->node->from;
    // node->leaf = p->node->leaf;
    
    // if (copyChildren)
    // {        
        // node->children = malloc(p->node->childrenCount * sizeof *(node->children));
        // if (!node->children)
            // debug("bad node children malloc");
    // }
    
    // return i;
// }

// int CopyBranchToSuffixTree(SuffixTree *st, int i, DfsPosition *p)
// {
    // i = AppendNodeToSuffixTree(st, i, p, 1), action;
    // NextStepOfDfs(p, 0);
    // int minDepth = p->node->depth;
    
    // while(action = NextStepOfDfs(p, minDepth != p->node->depth ? p->node->depth : minDepth)) 
    // {
        // if (action > 0)
            // i = AppendNodeToSuffixTree(st, i, p, 1);
        // else
            // i = st->nodes[i].parent;
    // }
    
    // // move position to next downward (node, edge)
    // NextStepOfDfs(p, minDepth);
    
    // return i;
// }

// void InitializeNextNodeInSuffixTree(SuffixTree *st, int parent, int suffixIndex, int depth, int leaf)
// {
    // AllocateNextNodeIndexInSuffixTree(st);
    // SuffixTreeNode *node = LastInSuffixTree(st);
    
    // node->parent = parent;
    // node->depth = depth;
    // node->from = suffixIndex + st->nodes[parent].depth;
    // node->leaf = leaf;
    
    // return st->count - 1;
// }

// void CompleteSuffixTreeNodeConstruction(SuffixTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer)
// {
    // int childrenCount = node->childrenCount = *LastInDynamicArray(childrenCountersBuffer);
    // childrenBuffer->count -= childrenCount;
    // if (childrenCount > 0)
        // CopyChildrenToNode(node, childrenBuffer->a + childrenBuffer->count, childrenCount);
    // --(childrenCountersBuffer->count);
// }

// void CopyChildrenToNode(SuffixTreeNode *node, int *children, int n)
// {
    // node->children = malloc(n * sizeof(int));
    // if (!node->children)
        // debug("bad node children malloc");
    
    // node->childrenCount = n;
    // if (children)
    // {
        // for (int i = 0; i < n; ++i)
            // node->children[i] = children[i];
    // }
// }

// int AppendNodeToSuffixTree(SuffixTree *st, int parent, DfsPosition *p, int copyChildren)
// {    
    // st->nodes[parent].children[*LastInDynamicArray(p->lastChild)] = AllocateNextNodeIndexInSuffixTree(st);
    // SuffixTreeNode *node = LastInSuffixTree(st);
    
    // node->parent = parent;
    // node->depth = srcNode->depth;
    // node->from = srcNode->from;
    // node->leaf = srcNode->leaf;
    
    // if (copyChildren)
        // CopyChildrenToNode(node, NULL, srcNode->childrenCount);
    
    // return st->count - 1;
// }
