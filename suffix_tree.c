SuffixTree *CreateSuffixTree()
{
    SuffixTree *st = calloc(1, sizeof *st);
    st->count = 0;
    st->capacity = 1;
    st->nodes = malloc(st->capacity * sizeof *st->nodes);
    if (!st->nodes)
        debug("bad nodes malloc in CreateSuffixTree");
        
    AllocateNextNodeIndexInSuffixTree(st);
}

void FreeSuffixTree(SuffixTree *st)
{
    for (int i = 0; i < st->count; i++)
        MemFree(st->nodes[i].children);
    MemFree(st->nodes);
    MemFree(st);
}

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
    MemFree(sa->lcp);
    MemFree(sa->a);
    MemFree(sa);
}

SuffixTree *CreateSuffixTreeFromSuffixArray(SuffixArray *sa)
{
    int n = sa->n;
    int *lcp = sa->lcp, *a = sa->a;
    
    SuffixTree *st = CreateSuffixTree();
    DynamicArray *childrenCountersBuffer = CreateDynamicArray(1), *childrenBuffer = CreateDynamicArray(1);
    int i, j;
    i = 0;
    
    j = InitializeNextNodeInSuffixTree(st, 0, 0, 0, -1);
    PushToDynamicArray(childrenCountersBuffer, 0);
    
    //add first leaf node
    j = InitializeNextNodeInSuffixTree(st, 0, a[0], n - a[0], a[0]);
    AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
    
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
            j = InitializeNextNodeInSuffixTree(st, j, a[i + 1], lcp[i], -1);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
        }
        else if (st->nodes[j].depth > lcp[i])
        {
            int child;
            do
            {
                CompleteNodeConstruction(&st->nodes[j], childrenCountersBuffer, childrenBuffer);
                child = j;
                j = st->nodes[j].parent;
            }
            while (st->nodes[j].depth > lcp[i]);
            
            if (st->nodes[j].depth < lcp[i])
            {
                //insert node;
                j = InitializeNextNodeInSuffixTree(st, j, a[i + 1], lcp[i], -1);
                st->nodes[child].parent = j;
                *LastInDynamicArray(childrenBuffer) = j;
                AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
            }
        }
        
        //add leaf node depth;
        if (lcp[i] < n - a[i + 1])
        {
            j = InitializeNextNodeInSuffixTree(st, j, a[i + 1], n - a[i + 1], a[i + 1]);
            AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
        }
        else
            st->nodes[j].leaf = a[i + 1];
    }
    
    //complete remaining nodes
    while (childrenCountersBuffer->count > 0)
    {
        CompleteNodeConstruction(&st->nodes[j], childrenCountersBuffer, childrenBuffer);
        j = st->nodes[j].parent;
    }
}

SuffixArray *CreateSuffixArrayFromSuffixTree(SuffixTree *st)
{
    return -1;
}

void AllocateNextNodeIndexInSuffixTree(SuffixTree *st)
{    
    int i = st->count;
    if (i == st->capacity)
    {        
        SuffixTreeNode *t = realloc(st->nodes, 2 * st->capacity * sizeof *st->units);
        if (t)
        {
            st->units = t;
            st->capacity = st->capacity << 1;
        }
        else
            debug("bad suffix tree units realloc");
    }
    st->nodes[i].children = NULL;
    ++st->count;
}

inline SuffixTreeNode *LastInSuffixTree(SuffixTree *st)
{
    return (st->nodes + st->count - 1);
}

inline int GetEdgeLength(dfsDepthsPosition *p)
{
    return p->tree->nodes[*LastInDynamicArray(p->lastChild)].depth - p->node->depth;
}

void InitializeNextNodeInSuffixTree(SuffixTree *st, int parent, int suffixIndex, int depth, int leaf)
{
    AllocateNextNodeIndexInSuffixTree(st);
    SuffixTreeNode *node = LastInSuffixTree(st);
    
    node->parent = parent;
    node->depth = depth;
    node->from = suffixIndex + st->nodes[parent].depth;
    node->leaf = leaf;
    
    return st->count - 1;
}

int AppendNodeToSuffixTree(SuffixTree *st, int parent, dfsDepthsPosition *p, int copyChildren)
{    
    st->nodes[parent].children[*LastInDynamicArray(p->lastChild)] = AllocateNextNodeIndexInSuffixTree(st);
    SuffixTreeNode *node = LastInSuffixTree(st);
    
    node->parent = parent;
    node->depth = srcNode->depth;
    node->from = srcNode->from;
    node->leaf = srcNode->leaf;
    
    if (copyChildren)
        CopyChildrenToNode(node, NULL, srcNode->childrenCount);
    
    return st->count - 1;
}

void CompleteNodeConstruction(SuffixTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer)
{
    int childrenCount = node->childrenCount = *LastInDynamicArray(childrenCountersBuffer);
    childrenBuffer->count -= childrenCount;
    if (childrenCount > 0)
        CopyChildrenToNode(node, childrenBuffer->a + childrenBuffer->count, childrenCount);
    --(childrenCountersBuffer->count);
}

void CopyChildrenToNode(SuffixTreeNode *node, int *children, int n)
{
    node->children = malloc(n * sizeof(int));
    if (!node->children)
        debug("bad node children malloc");
    
    node->childrenCount = n;
    if (children)
    {
        for (int i = 0; i < n; ++i)
            node->children[i] = children[i];
    }
}

void AddChildToBufferAndMakeItCurrent(DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer, int j)
{
    //add child
    ++(childrenCountersBuffer->count);
    PushToDynamicArray(childrenBuffer, j);
    
    //initialize new counter for this child
    PushToDynamicArray(childrenCountersBuffer, 0):
}


// --------------------------EULER TOUR FOR SUFFIX ARRAY -----------------------
SuffixArrayEulerTour *CreateSuffixArrayEulerTour(int n, DynamicArray *dfsDepths, int *rankToDfsDepths, int *suffixToRank)
{    
    SuffixArrayEulerTour *eulerTour = calloc(1, sizeof *eulerTour);
    eulerTour->n = n;
    eulerTour->dfsDepths = dfsDepths;
    eulerTour->rankToDfsDepths = rankToDfsDepths;
    eulerTour->suffixToRank = suffixToRank;
}

void FreeSuffixArrayEulerTour(SuffixArrayEulerTour *eulerTour)
{
    debug("free EulerTour: started");
    MemFree(eulerTour->rankToDfsDepths);
    FreeDynamicArray(eulerTour->dfsDepths);
    MemFree(eulerTour->suffixToRank);
    MemFree(eulerTour);
    debug("free EulerTour: finished");
}

SuffixArrayEulerTour *GetSuffixArrayEulerTour(SuffixArray *sa, int *suffixToRank)
{
    debug("create EulerTour for SuffixArray: started");
    
    int *lcp = sa->lcp;
    int i, d, n;
    i = d = 0;
    n = sa->n;
 
    int *rankToDfsDepths = malloc(n * sizeof *rankToDfsDepths);
    DynamicArray *dfsDepths = CreateDynamicArray(2 * (n + 1));
                
    while (i < n)
    {
        while (d <= lcp[i])
        {
            AllocateNextIndexInDynamicArray(dfsDepths);
            *TopInDynamicArray(dfsDepths) = d++;
        }        
        rankToDfsDepths[i] = dfsDepths->count;        
        do
        {
            AllocateNextIndexInDynamicArray(dfsDepths);
            *TopInDynamicArray(dfsDepths) = d--;
        }
        while (d >= lcp[i]);        
        d += 2;        
        ++i;
    }
    
    d -= 2;
    while (d >= 0)
    {
        AllocateNextIndexInDynamicArray(dfsDepths);
        *TopInDynamicArray(dfsDepths) = d--;
    }
    
    debugArr(dfsDepths->a, dfsDepths->count);
    debugArr(rankToDfsDepths, n);
    
    debug("create EulerTour for SuffixArray: finished");
    
    return CreateSuffixArrayEulerTour(n, dfsDepths, rankToDfsDepths, suffixToRank);
}

int GetLcpForSuffixArray(LcaTable *lcaTable, SuffixArrayEulerTour *eulerTour, int v1, int v2) 
{
    if (v1 >= eulerTour->n || v2 >= eulerTour->n)
        return 0;
        
    printf("(%d, %d) ", v1, v2);
    fflush(stdout);
    
    int nv1 = eulerTour->suffixToRank[v1], nv2 = eulerTour->suffixToRank[v2];
    printf("(%d, %d) ", nv1, nv2);
    fflush(stdout);
    
	int l = eulerTour->rankToDfsDepths[nv1];
    int r = eulerTour->rankToDfsDepths[nv2];        
	if (l > r)
        Swap(&l, &r);
        
    return GetLcp(lcaTable, l, r);
}