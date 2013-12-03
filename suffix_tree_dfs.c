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
    return p->tree->nodes[p->node->children[*LastInDynamicArray(p->lastChild)]].depth - p->node->depth;
}

inline int GetFirstCharOfChildEdge(DfsPosition *p)
{
    return (*LastInDynamicArray(p->lastChild) < p->node->childrenCount) 
        ? p->s[p->tree->nodes[GetChildIndex(p)].from]
        : p->s[p->node->leaf + p->node->depth];
}

inline int GetChildIndex(DfsPosition *p)
{
    return p->node->children[*LastInDynamicArray(p->lastChild)];
}
/* --------------------------------------------- */

void SwapPositions(DfsPosition **px, DfsPosition **py)
{
    DfsPosition *t = *px;
    *px = *py;
    *py = t;
}

int CompareAndSwapDfsPositions(DfsPosition **ppx, DfsPosition **ppy)
{
    DfsPosition *px = *ppx, *py = *ppy;
    
    if (px->node->depth == py->node->depth)
    {
        int cx = GetFirstCharOfChildEdge(px);
        int cy = GetFirstCharOfChildEdge(py);
        
        if (cx < cy)
            SwapPositions(&px, &py);
        return cx != cy;
    }
    else 
    {
        if (px->node->depth < py->node->depth)
            SwapPositions(ppx, ppy);            
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