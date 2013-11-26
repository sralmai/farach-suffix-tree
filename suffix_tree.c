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
    MemFreeDfs(st);
    MemFree(st->nodes);
}

SuffixTree *CreateSuffixTreeFromSuffixArray(SuffixArray *suffixArray)
{
    int n = suffixArray->n;
    int *lcp = suffixArray->lcp, *a = suffixArray->a;
    
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

inline int GetEdgeLength(DfsPosition *p)
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

int AppendNodeToSuffixTree(SuffixTree *st, int parent, DfsPosition *p, int copyChildren)
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