#include <stdlib.h>
#include "suffix_tree_dfs.h"
#include "helpers.h"

DfsPosition *CreateDfsPosition(SuffixTree *st, int *s, int n, int ind, int treeType)
{
    DfsPosition *p = calloc(1, sizeof *p);
    p->tree = st;
    p->treeType = treeType;
    p->s = s;
    p->n = n;
    p->ind = ind;
    p->lastDfsLeaf = 0;
    
    p->lastChild = CreateDynamicArray(1);
    PushToDynamicArray(p->lastChild, 0);
    
    return p;
}

void FreeDfsPosition(DfsPosition *p)
{
    if (!p)
        return;
        
    FreeDynamicArray(p->lastChild);
    MemFree(p);
}

/* ------------ short inline helpers ------------ */
inline int EndOfDfs(DfsPosition *p)
{
    return 0 == p->ind && *LastInDynamicArray(p->lastChild) == p->tree->nodes[0].childrenCount;
}

inline int GetEdgeLength(DfsPosition *p)
{
    return p->tree->nodes[GetChildIndex(p)].depth - p->tree->nodes[p->ind].depth;
}

inline int GetFirstCharOfChildEdge(DfsPosition *p)
{
    return *LastInDynamicArray(p->lastChild) < p->tree->nodes[p->ind].childrenCount
        ? p->s[p->tree->nodes[GetChildIndex(p)].from]
        : p->s[p->n];
}

inline int GetChildIndex(DfsPosition *p)
{
    return p->tree->nodes[p->ind].children[*LastInDynamicArray(p->lastChild)];
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
    
    if (px->tree->nodes[px->ind].depth == py->tree->nodes[py->ind].depth)
    {
        int cx = GetFirstCharOfChildEdge(px);
        int cy = GetFirstCharOfChildEdge(py);
        
        if (cx > cy)
            SwapPositions(ppx, ppy);
        return cx != cy;
    }
    else 
    {
        if (px->tree->nodes[px->ind].depth < py->tree->nodes[py->ind].depth)
            SwapPositions(ppx, ppy);            
        return 1;
    }
}

int NextStepOfDfs(DfsPosition *p, int minDepth)
{
    if (*LastInDynamicArray(p->lastChild) < p->tree->nodes[p->ind].childrenCount)
    {
        // move down through next child edge
        p->ind = GetChildIndex(p);
        PushToDynamicArray(p->lastChild, 0);
    }
    // go upward until we can go downward in next step
    while (*LastInDynamicArray(p->lastChild) >= p->tree->nodes[p->ind].childrenCount)
    {
        if (p->ind == 0 || p->tree->nodes[p->tree->nodes[p->ind].parent].depth < minDepth)
            return 0;
        
        // up to parent
        p->ind = p->tree->nodes[p->ind].parent;
        --(p->lastChild->count);
        ++(*LastInDynamicArray(p->lastChild));
    }
    return 1;
}
