typedef struct _dfsPosition
{
    SuffixTree *tree;
    int treeType;
    //string
    int *s;
    
    //current node
    int ind;
    SuffixTreeNode *node;
    
    //children indexes in path
    DynamicArray *lastChild;
} DfsPosition;

void FreeDfsPosition(DfsPosition *p)
{
    FreeDynamicArray(lastChild);
    MemFree(p);
}

DfsPosition *CreateDfsPosition(SuffixTree *st, int *s, int ind, int treeType)
{
    DfsPosition *p = calloc(1, sizeof *p);
    p->tree = st;
    p->treeType = treeType;
    p->s = s;
    p->ind = ind;
    p->node = p->tree->nodes[ind];
    
    p->lastChild = CreateDynamicArray(1);
    PushToDynamicArray(p->lastChild, 0);
    return p;
}

int NextStepOfDfs(DfsPosition *p, int minDepth)
{
    if (*LastInDynamicArray(p->lastChild) < p->node->childrenCount)
    {
        // move down through next child edge
        p->ind = p->node->children[*LastInDynamicArray(p->lastChild)];
        p->node = GetCurrentNode(p);
        PushToDynamicArray(p->lastChild, 0);
    }    
    //check if we can go downward in next step    
    while (*LastInDynamicArray(p->lastChild) >= p->node->childrenCount)
    {
        if (p->node->depth <= minDepth)
            return 0;
        
        // up to parent
        p->ind = pos->node->parent;
        p->node = GetCurrentNode(p);
        --(p->lastChild->count);
        ++(*LastInDynamicArray(p->lastChild));
    }    
    return 1;    
}

inline int GetChildIndex(DfsPosition *p)
{
    return p->node->children[*LastInDynamicArray(p->lastChild)];
}

inline int EndOfDfs(DfsPosition *p)
{
    return 0 == p->node->depth && *LastInDynamicArray(p->lastChild) == p->node->childrenCount;
}

int CopyBranchToSuffixTree(SuffixTree *st, int i, DfsPosition *p)
{
    i = AppendNodeToSuffixTree(st, i, p, 1), action;
    NextStepOfDfs(p, 0);
    int minDepth = p->node->depth;
    
    while(action = NextStepOfDfs(p, minDepth != p->node->depth ? p->node->depth : minDepth)) 
    {
        if (action > 0)
            i = AppendNodeToSuffixTree(st, i, p, 1);
        else
            i = st->nodes[i].parent;
    }
    
    // move position to next downward (node, edge)
    NextStepOfDfs(p, minDepth);
    
    return i;
}

inline int GetFirstCharOfChildEdge(DfsPosition *p)
{
    return (*LastInDynamicArray(p->lastChild) < p->node->childrenCount) 
        ? s[p->tree->nodes[GetChildIndex(p)].from]
        : 0;
}

int CompareAndSwapDfsPositions(DfsPosition **px, DfsPosition **py)
{   
    if (px->node->depth == py->node->depth)
    {
        px->c = GetFirstCharOfChildEdge(px);
        py->c = GetFirstCharOfChildEdge(py);
        
        if (py->c < px->c)
            Swap(px, py);            
        return px->c < py->c;
    }
    else 
    {
        if (px->node->depth < py->node->depth)
            Swap(px, py);            
        return 1;
    }
}

inline int GetEdgeLength(DfsPosition *p)
{
    return p->tree->nodes[GetChildIndex(p)].depth - p->node->depth;
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

void CompleteSuffixTreeNodeConstruction(SuffixTreeNode *node, DynamicArray *childrenCountersBuffer, DynamicArray *childrenBuffer)
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

void BreakSuffixTreeEdgeByCustomLength(SuffixTree *tree, int currentNodeIndex, int childOrderNumber, int edgeLen)
{
    SuffixTreeNode *node = tree->nodes[currentNodeIndex];
    int child = node->children[childOrderNumber];
    int childFrom = tree->nodes[child].from;
    int newChildDepth = node->depth + edgeLen;
    
    int newNode = InitializeNextNodeInSuffixTree(tree, currentNodeIndex, childFrom, newChildDepth, -1);
    tree->nodes[child].parent = newNode;
    tree->nodes[child].from = childFrom + newChildDepth;
    
    px->node->children[childOrderNumber] = newNode;
}