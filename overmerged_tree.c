typedef struct _overMegedTreeNode
{
    int parent;
    int oddIndex, evenIndex;
    
    int *children;
    int childrenCount;
    
    SuffixTree *evenTree, *oddTree;
} OverMergedTreeNode;

typedef struct _overMegedTree
{
    int count, capacity;
    OverMergedTreeNode *nodes;
} OverMergedTree;

OverMergedTree *CreateOverMergedTree()
{
    OverMergedTree *omt = calloc(1, sizeof *omt);
    omt->count = 0;
    omt->capacity = 1;
    omt->nodes = malloc(omt->capacity * sizeof *omt->nodes);
    if (!omt->nodes)
        debug("bad nodes malloc in OverMergeTrees");        
    AllocateNextNodeIndexInOverMergedTree(omt);    
    
    return omt;
}

void FreeOverMergedTree(OverMergedTree *omt)
{
    MemFreeDfs(omt);
    MemFree(omt->nodes);
}

int AllocateNextNodeIndexInOverMergedTree(OverMergedTree *omt)
{    
    int i = omt->count;
    if (i == omt->capacity)
    {        
        OverMergedTreeNode *t = realloc(omt->nodes, 2 * omt->capacity * sizeof *omt->units);
        if (t)
        {
            omt->units = t;
            omt->capacity = omt->capacity << 1;
        }
        else
            debug("bad overmerged tree units realloc");
    }
    omt->nodes[i].children = NULL;
    ++omt->count;
    return i;
}

int AppendNodeToOverMergedTree(OverMergedTree *omt, int parent, DfsPosition *px, DfsPosition *py)
{    
    int nodeIndex = AllocateNextNodeIndexInOverMergedTree(omt);
    OverMergedTreeNode *node = omt->nodes[nodeIndex];
    node->parent = parent;
    node->depth = px->node->depth;
    
    if (0 == px->treeType)
    {
        node->evenIndex = px->ind;
        node->oddIndex = py ? py->ind : -1;
    }
    else
    {
        node->oddIndex = px->ind;
        node->evenIndex = py ? py->ind : -1;
    }
    return nodeIndex;
}

inline int GetOddOrEvenIndex(OverMergedTree *omt, int i)
{
    return omt->nodes[i].evenIndex != -1 ? omt->nodes[i].evenIndex : omt->nodes[i].oddTree;
}

OverMergedTree *OverMergeTrees(SuffixTree *evenTree, SuffixTree *oddTree, int *s, int n)
{
    OverMergedTree *omt = CreateOverMergedTree();
    
    DynamicArray *childrenCountersBuffer = CreateDynamicArray(1), *childrenBuffer = CreateDynamicArray(1);    
    DfsPosition *px = CreateDfsPosition(evenTree, s, 0, 0), *py = CreateDfsPosition(oddTree, s, 0, 1);
    
    // euler tour structures
    DynamicArray *dfsToIndex = CreateDynamicArray(1), *dfs = CreateDfsPosition(1);
    int *appearance = malloc(n * sizeof *appearance);
    int depth = 0;
    // ---------------------
        
    // create root node
    int i = AppendNodeToOverMergedTree(omt, 0, px, py), oddEvenNodesCount = 0;
    PushToDynamicArray(childrenCountersBuffer, 0);
    
    // put root to odd/even for clarity
    PushToDynamicArray(dfs, depth++);
    PushToDynamicArray(dfsToIndex, i);
    // --------------------------------
    
    while (!EndOfDfs(px) || !EndOfDfs(py))
    {        
        if (CompareAndSwapDfsPositions(&px, &py))
        {
            // we can copy px branch, but copy only it's root
            i = AppendNodeToOverMergedTree(omt, i, px, py);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, i);            
            
            // px children hack
            NextStepOfDfs(px);
            *LastInDynamicArray(px->lastChild) = px->node->childrenCount;
            NextStepOfDfs(px);
            // now it point to next step after child we've added, i.e. we skipped whole inner branch
            
            // this node is root of subbranch of odd or even tree
            appearance[GetOddOrEvenIndex(omt, i)] = i;
            PushToDynamicArray(dfs, depth++);
            PushToDynamicArray(dfsToIndex, i);
            // -----------------------------------------------------------
        }
        else
        {
            int lx = GetEdgeLength(px), ly = GetEdgeLength(py);
            
            if (lx < ly)
            {
                Swap(&px, &py);
                Swap(&lx, &ly);
            }
                        
            if (lx > ly)
            {
                //insert node in px->tree
                BreakSuffixTreeEdgeByCustomLength(px->tree, px->ind, *LastInDynamicArray(px->lastChild), ly);
            }
            
            //merge edges
            i = AppendNodeToOverMergedTree(omt, i, px, py);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, i);
            oddEvenNodesCount++;
            
            // added node is odd/even ==> don't touch appearance
            PushToDynamicArray(dfs, depth++);
            PushToDynamicArray(dfsToIndex, i);
            // -------------------------------------------------
            
            NextStepOfDfs(px, 0);
            NextStepOfDfs(py, 0);
        }
        
        
        while (omt->nodes[i].depth > px->node->depth && omt->nodes[i].depth > py->nodes->depth)
        {   
            CompleteNodeConstruction(&omt->nodes[i], childrenCountersBuffer, childrenBuffer);
            i = omt->nodes[i].parent;
            
            // go upward
            PushToDynamicArray(dfs, --depth);
        }
    }
    
    if (i != 0)
        debug("воу, воу, палехчи");        
        
    // complete root's children building
    CompleteNodeConstruction(&omt->nodes[0], childrenCountersBuffer, childrenBuffer);
    omt->oddEvenNodesCount = oddEvenNodesCount;
    omt->oddTree = oddTree;
    omt->evenTree = evenTree;
    
    return omt;
}

SuffixTree *BuildSuffixTree(OverMergedTree *omt, LcpTable *lcp)
{
    SuffixTree *oddTree = omt->oddTree, *evenTree = omt->evenTree;
    SuffixTree *st = CreateSuffixTree();
    int i = 0;
    
    while (stack isn't empty)
    {
        i = pop from stack
        
        if (odd/even)
        {
            j = GetLcp(ei + 1, oi + 1);
            if (dj + 1 < d)
            {
                break edge;
                build left branch;
                build right branch;
            }
        }
        else
        {
            copy whole branch
        }
    }
}