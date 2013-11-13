#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include "helpers.h"
#include "prefix_tree.h"

/* ----------- Prefix tree -------------- */

void InitializePrefixTree(PrefixTree *pTree, int capacity)
{
    debug("initialize prefix tree");
    
    pTree->count = 0;
    pTree->capacity = capacity;
    pTree->units = malloc(capacity * sizeof *pTree->units);
    if (!pTree->units)
        debug("bad malloc in InitializePrefixTree");
    else
        printf("struct size: %d\n", (int)sizeof *pTree->units);
    
    AllocateNextIndexInPrefixTree(pTree);
}

void FreePrefixTree(PrefixTree *pTree)
{
    debug("free prefix tree");
    
    pTree->count = 0;
    pTree->capacity = 0;
    MemFree(pTree->units);
}

void AllocateNextIndexInPrefixTree(PrefixTree *pTree)
{    
    int i = pTree->count;
    if (i == pTree->capacity)
    {        
        PrefixTreeUnit *t = realloc(pTree->units, 2 * pTree->capacity * sizeof *pTree->units);
        if (t)
        {
            pTree->units = t;
            pTree->capacity = pTree->capacity << 1;
            printf("==> new units array size: %d bytes\n", (int)(pTree->capacity * sizeof *t));
        }
        else
            debug("bad units realloc");
    }
    
    pTree->units[i].rank = -1;
    for (int j = 0; j < EngAlphabetCardinality; j++)
        pTree->units[i].children[j] = -1;
    pTree->count++;
}

void AddLetterToPrefixTree(PrefixTree *pTree, char *s, int n)
{    
    int ind = 0;
    int childUnitIndex = -1;
    int c;
    
    for (int i = 0; i < n; i++)
    {
        c = s[i] - 'A';
        assert(c >= 0);
        
        childUnitIndex = pTree->units[ind].children[c];
        if (childUnitIndex < 0)
        {
            childUnitIndex = pTree->count;
            AllocateNextIndexInPrefixTree(pTree);
            
            pTree->units[ind].children[c] = childUnitIndex;
        }
        ind = childUnitIndex;
    }
    
    pTree->units[ind].rank = 1;
}

void CompleteBuildingPrefixTree(PrefixTree *pTree, int i, int *rank)
{    
    if (1 == pTree->units[i].rank)
        pTree->units[i].rank = (*rank)++;
        
    for (int j = 0; j < EngAlphabetCardinality; j++)
    {
        if (pTree->units[i].children[j] >= 0)
            CompleteBuildingPrefixTree(pTree, pTree->units[i].children[j], rank);
    }
}

int GetIndexOfLetterInPrefixTree(PrefixTree *pTree, char *s, int n)
{
    int c, ind = 0;
    
    for (int i = 0; i < n; i++)
    {
        c = s[i] - 'A';
        assert(c >= 0);
        
        if (pTree->units[ind].children[c] >= 0)
            ind = pTree->units[ind].children[c];
        else
            return -1;
    }
    return pTree->units[ind].rank;
}