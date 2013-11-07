/* prefix tree */

#define EngAlphabetCardinality 28

typedef struct _prefixTreeUnit
{
    int rank;
    int children[EngAlphabetCardinality];
} PrefixTreeUnit;

typedef struct _prefixTree
{
	int count;
    int arrLength;
	PrefixTreeUnit *units;
} PrefixTree;

void InitializePrefixTree(PrefixTree *pTree, int size);
void FreePrefixTree(PrefixTree *pTree);
int GetNextPrefixTreeUnitIndex(PrefixTree *pTree);
void AddLetterToPrefixTree(PrefixTree *pTree, char *s, int len);
void CompletePrefixTreeBuild(PrefixTree *pTree, int i, int *rank);
int GetIndexOfLetter(PrefixTree *pTree, char *s, int len);

/* suffix tree */

typedef struct _suffixTreeUnit
{
    int parent;
} SuffixTreeUnit;

typedef struct _suffixTreeUnit
{
    int n;
    int *lcp;
    int *a;
}