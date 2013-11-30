#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "helpers.h"
#include "prefix_tree.h"

/* ----------- Prefix tree -------------- */

void InitializePrefixTree(PrefixTree *pTree, int capacity)
{
    debug("initialize prefix tree");
    
    pTree->count = 0;
    pTree->capacity = capacity;
    pTree->nodes = malloc(capacity * sizeof *pTree->nodes);
    if (!pTree->nodes)
        debug("bad malloc in InitializePrefixTree");
    else
        printf("struct size: %d\n", (int)sizeof *pTree->nodes);
    
    AllocateNextIndexInPrefixTree(pTree);
}

void FreePrefixTree(PrefixTree *pTree)
{
    debug("free prefix tree");
    
    pTree->count = 0;
    pTree->capacity = 0;
    MemFree(pTree->nodes);
}

void AllocateNextIndexInPrefixTree(PrefixTree *pTree)
{    
    int i = pTree->count;
    if (i == pTree->capacity)
    {        
        PrefixTreeNode *t = realloc(pTree->nodes, 2 * pTree->capacity * sizeof *pTree->nodes);
        if (t)
        {
            pTree->nodes = t;
            pTree->capacity = pTree->capacity << 1;
        }
        else
            debug("bad prefix tree nodes realloc");
    }
    
    pTree->nodes[i].rank = -1;
    for (int j = 0; j < EngAlphabetCardinality; j++)
        pTree->nodes[i].children[j] = -1;
    pTree->count++;
}

void AddLetterToPrefixTree(PrefixTree *pTree, char *s, int n)
{    
    int ind = 0;
    int childIndex = -1;
    int c;
    
    for (int i = 0; i < n; i++)
    {
        c = s[i] - 'A';
        assert(c >= 0);
        
        childIndex = pTree->nodes[ind].children[c];
        if (childIndex < 0)
        {
            childIndex = pTree->count;
            AllocateNextIndexInPrefixTree(pTree);
            
            pTree->nodes[ind].children[c] = childIndex;
        }
        ind = childIndex;
    }
    
    pTree->nodes[ind].rank = 1;
}

void CompleteBuildingPrefixTree(PrefixTree *pTree, int i, int *rank)
{    
    if (1 == pTree->nodes[i].rank)
        pTree->nodes[i].rank = (*rank)++;
        
    for (int j = 0; j < EngAlphabetCardinality; j++)
    {
        if (pTree->nodes[i].children[j] >= 0)
            CompleteBuildingPrefixTree(pTree, pTree->nodes[i].children[j], rank);
    }
}

int GetIndexOfLetterInPrefixTree(PrefixTree *pTree, char *s, int n)
{
    int c, ind = 0;
    
    for (int i = 0; i < n; i++)
    {
        c = s[i] - 'A';
        assert(c >= 0);
        
        if (pTree->nodes[ind].children[c] >= 0)
            ind = pTree->nodes[ind].children[c];
        else
            return -1;
    }
    return pTree->nodes[ind].rank;
}