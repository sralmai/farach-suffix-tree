/* prefix tree */

#define EngAlphabetCardinality 28

typedef struct _prefixTreeNode
{
    int rank;
    int children[EngAlphabetCardinality];
} PrefixTreeNode;

typedef struct _prefixTree
{
	int count, capacity;
	PrefixTreeNode *nodes;
} PrefixTree;

void InitializePrefixTree(PrefixTree *pTree, int capacity);
void FreePrefixTree(PrefixTree *pTree);
void AllocateNextIndexInPrefixTree(PrefixTree *pTree);
void AddLetterToPrefixTree(PrefixTree *pTree, char *s, int n);
void CompleteBuildingPrefixTree(PrefixTree *pTree, int i, int *rank);
int GetIndexOfLetterInPrefixTree(PrefixTree *pTree, char *s, int n);