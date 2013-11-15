SuffixTree *BuildSuffixTreeForSuffixArray(SuffixArray *suffixArray)
{
    int n = suffixArray->n;
    int *lcp = suffixArray->lcp, *a = suffixArray->a;
    
    SuffixTree *st = GetDegenerateSuffixTree();
    DynamicArray *childrenCountersBuffer = CreateDynamicArray(1), *childrenBuffer = CreateDynamicArray(1);
    int i, j, prefixLength;
    i = 0;
    
    j = InitializeNextNodeInSuffixTree(st, 0, 0, 0, -1);
    PushToDynamicArray(childrenCountersBuffer, 0);
    
    //add first leaf node
    j = InitializeNextNodeInSuffixTree(st, 0, a[0], n - a[0], a[0]);
    AddChildToBufferAndMakeItCurrent(childrenCountersBuffer, childrenBuffer, j);
    
    while (i < n)
    {
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

SuffixTree *GetDegenerateSuffixTree()
{
    SuffixTree *st = calloc(1, sizeof *st);
    st->count = 0;
    st->capacity = 1;
    st->nodes = malloc(st->capacity * sizeof *st->nodes);
    if (!st->nodes)
        debug("bad nodes malloc in GetDegenerateSuffixTree");
        
    AllocateNextNodeIndexInSuffixTree(st);
}

void FreeSuffixTree(SuffixTree *st)
{
    MemFreeDfs(st);
    MemFree(st->nodes);
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

void InitializeNextNodeInSuffixTree(SuffixTree *st, int parent, int suffixIndex, int depth, int leaf)
{
    AllocateNextNodeIndexInSuffixTree();
    SuffixTreeNode *node = LastInSuffixTree(st);
    
    node->parent = parent;
    node->depth = depth;
    node->from = suffixIndex + st->nodes[parent].depth;
    node->leaf = leaf;
    
    return st->count - 1;
}

void CompleteNodeConstruction(SuffixTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer)
{
    int childrenCount = *LastInDynamicArray(childrenCountersBuffer);
    if (childrenCount > 0)
    {
        node->children = malloc(childrenCount * sizeof(int));
        if (!node->children)
            debug("bad node children malloc");
        childrenBuffer->count -= childrenCount;
        
        for (int i = 0, base = childrenBuffer->count; i < childrenCount; ++i)
            node->children[i] = childrenBuffer->a[base + i];
    }
    --(childrenCountersBuffer->count);
}

void AddChildToBufferAndMakeItCurrent(DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer, int j)
{
    //add child
    ++(childrenCountersBuffer->count);
    PushToDynamicArray(childrenBuffer, j);
    
    //initialize new counter for this child
    PushToDynamicArray(childrenCountersBuffer, 0):
}