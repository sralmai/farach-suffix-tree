#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include "helpers.h"
#include "lca_algo.h"

EulerTour *GetEulerTour(SuffixTree *tree)
{
    debug("create EulerTour: started");
    
    int *lcp = tree->lcp;
    int i, d, n;
    i = d = 0;
    n = tree->n;
 
    int *appearance = malloc(n * sizeof *appearance);
    DynamicArray *dfs = CreateDynamicArray(2 * (n + 1));
                
    while (i < n)
    {
        while (d <= lcp[i])
        {
            AllocateNextIndexInDynamicArray(dfs);
            *TopInDynamicArray(dfs) = d++;
        }
        
        appearance[i] = dfs->count;
        
        do
        {
            AllocateNextIndexInDynamicArray(dfs);
            *TopInDynamicArray(dfs) = d--;
        }
        while (d >= lcp[i]);        
        d += 2;
        
        i++;
    }
    
    d -= 2;
    while (d >= 0)
    {
        AllocateNextIndexInDynamicArray(dfs);
        *TopInDynamicArray(dfs) = d--;
    }
    
    debugArr(dfs->a, dfs->count);
    debugArr(appearance, n);
    
    EulerTour *eulerTour = calloc(1, sizeof *eulerTour);
    eulerTour->n = n;
    eulerTour->dfs = dfs;
    eulerTour->appearance = appearance;
    
    debug("create EulerTour: finished");
    return eulerTour;
}

void FreeEulerTour(EulerTour *eulerTour)
{
    debug("free EulerTour: started");
    MemFree(eulerTour->appearance);
    FreeDynamicArray(eulerTour->dfs);
    MemFree(eulerTour);
    debug("free EulerTour: finished");
}

int *allocate2D(int ***arr, int n, int m)
{
    *arr = malloc(n * sizeof **arr);
    int *arr_data = malloc(n * m * sizeof *arr_data);
    
    for(int i = 0; i < n; i++)
        (*arr)[i] = arr_data + i * m;
        
    return arr_data; //free point
} 

int *allocate3D(int ****arr, int ***arr2d, int n, int m, int k)
{
    *arr = malloc(n * sizeof **arr);
    *arr2d = malloc(n * m * sizeof **arr2d);
    int *arr_data = malloc(n * m * k * sizeof *arr_data);
    
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
            (*arr2d)[i * m + j] = arr_data + (i * m  + j) * k;
        (*arr)[i] = (*arr2d) + i * m;
    }
        
    return arr_data; //free point
} 

// compares two indices in a
inline int MinDepth(DynamicArray *dfs, int i, int j) 
{
	return dfs->a[i] <= dfs->a[j] ? i : j;
}

LcpTable *CreateLcpTable(SuffixTree *tree)
{
    debug("create LcpTable: started");
    
    int n = tree->n;
    int *suffixToIndex = malloc(n * sizeof *suffixToIndex);
    for (int i = 0; i < n; i++)
        suffixToIndex[tree->a[i] / 2] = i;
    
    
    LcpTable *lcpTable = calloc(1, sizeof *lcpTable);
    lcpTable->suffixToIndex = suffixToIndex;
    lcpTable->suffixTree = tree;
    lcpTable->eulerTour = GetEulerTour(tree);
    
    debug("create LcpTable: finished");
    return lcpTable;
}

void BuildLcpTable(LcpTable *lcpTable)
{
    /* 
        n = dfs array length
        blockSize = size of block  (Log2(n) upper bounded)
        blocks = blocks count
        sparseTable = array[blocks][Log2(n) + 1] initialized with -1
        blocksHash = array[blocks] initialized with 0
        rmqTable = array[sqrtN][blockSize][blockSize] initialized with -1
    */
    
    DynamicArray *dfs = lcpTable->eulerTour->dfs;
	int n = dfs->count, log2n = Log2(n);
	int blockSize = (log2n + 1) / 2;
	int blocks = n / blockSize + (n % blockSize ? 1 : 0);
    int **sparseTable, *sparseTableData;
    sparseTableData = allocate2D(&sparseTable, blocks, log2n + 1);

	// precalculate in each block and build sparse table
	memset(sparseTableData, -1, blocks * (log2n + 1) * sizeof *sparseTableData);
	for (int i = 0, bl = 0, j = 0; i < n; ++i, ++j) 
    {
		if (j == blockSize)
        {
			j = 0;
            ++bl;
        }
		if (sparseTable[bl][0] == -1 || MinDepth(dfs, i, sparseTable[bl][0]) == i)
			sparseTable[bl][0] = i;
	}
    
	for (int j = 1; j <= log2n; ++j)
    {
		for (int i = 0; i < blocks; ++i) 
        {
			int ni = i + (1 << (j - 1));
			if (ni >= blocks)
				sparseTable[i][j] = sparseTable[i][j - 1];
			else
				sparseTable[i][j] = MinDepth(dfs, sparseTable[i][j - 1], sparseTable[ni][j - 1]);
		}
    }

	// calculate hashes of blocks
    // calloc zeroes array
    int *blocksHash = calloc(blocks, sizeof *blocksHash);
    int smthLikeSqrtN = 0;
    
	for (int i = 0, bl = 0, j = 0; i < n || j < blockSize; ++i, ++j) 
    {
		if (j == blockSize)
        {
			j = 0;
            ++bl;
        }
        
		if (j > 0 && (i >= n || MinDepth(dfs, i - 1, i) == i - 1))
			blocksHash[bl] += 1 << (j - 1);
            
        if (blocksHash[bl] > smthLikeSqrtN)
            smthLikeSqrtN = blocksHash[bl];
	}

    ++smthLikeSqrtN;
    
	// precalculate RMQ inside each unique block
    int ***rmqTable, **rmq2dTables, *rmqTableData;
    rmqTableData = allocate3D(&rmqTable, &rmq2dTables, smthLikeSqrtN, blockSize, blockSize);
	memset(rmqTableData, -1, smthLikeSqrtN * blockSize * blockSize * sizeof *rmqTableData);
    
    // printf("blocks = %d, blockSize = %d\n", blocks, blockSize);
    // fflush(stdout);
    
	for (int i = 0; i < blocks; ++i) 
    {
		int id = blocksHash[i];

		if (rmqTable[id][0][0] != -1)  
            continue;
            
		for (int l = 0; l < blockSize; ++l) 
        {
			rmqTable[id][l][l] = l;
		
            for (int r = l + 1; r < blockSize; ++r) 
            {
				rmqTable[id][l][r] = rmqTable[id][l][r - 1];
                
				if (i * blockSize + r < n)
					rmqTable[id][l][r] =
						MinDepth(dfs, i * blockSize + rmqTable[id][l][r], i * blockSize + r) - i * blockSize;
			}
		}
	}

    // for (int i = 0; i < smthLikeSqrtN; i++)
        // for (int j = 0; j < blockSize; j++)
            // debugArr(rmqTable[i][j], blockSize);
    // fflush(stdout);
    
	// precalc logarithms: [i] = Log2(i) lower bounded
    int *precalcLog2 = malloc(n * sizeof *precalcLog2);
	for (int i = 0, j = 0; i < n; ++i) 
    {
		if (1 << (j + 1) <= i)
            ++j;
		precalcLog2[i] = j;
	}
        
	lcpTable->blockSize = blockSize;
    lcpTable->blocks = blocks;
    lcpTable->sparseTable = sparseTable;
    lcpTable->sparseTableData = sparseTableData;
    lcpTable->blocksHash = blocksHash;
    lcpTable->rmqTable = rmqTable;
    lcpTable->rmq2dTables = rmq2dTables;
    lcpTable->rmqTableData = rmqTableData;
    lcpTable->precalcLog2 = precalcLog2;
}

// answers RMQ in block #bl [l;r] in O(1)
inline int GetLcpInBlock(LcpTable *lcpTable, int bl, int l, int r) 
{
	return lcpTable->rmqTable[lcpTable->blocksHash[bl]][l][r] + bl * lcpTable->blockSize;
}

// answers LCA in O(1)
int GetLcp(LcpTable *lcpTable, int v1, int v2) 
{
    if (v1 >= lcpTable->suffixTree->n || v2 >= lcpTable->suffixTree->n)
        return 0;
        
    printf("(%d, %d) ", v1, v2);
    fflush(stdout);
    
    int nv1 = lcpTable->suffixToIndex[v1], nv2 = lcpTable->suffixToIndex[v2];
    printf("(%d, %d) ", nv1, nv2);
    fflush(stdout);
    
	int l = lcpTable->eulerTour->appearance[nv1];
    int r = lcpTable->eulerTour->appearance[nv2];
    DynamicArray *dfs = lcpTable->eulerTour->dfs;
    int blockSize = lcpTable->blockSize;
        
	if (l > r)
    {
        // swap it
        int temp = l;
        l = r;
        r = temp;
    }
    
    printf("(%d, %d)\n", l, r);
    fflush(stdout);
    
	int bl = l / blockSize, br = r / blockSize;
    
	if (bl == br)
		return dfs->a[GetLcpInBlock(lcpTable, bl, l % blockSize, r % blockSize)];
        
	int ans1 = GetLcpInBlock(lcpTable, bl, l % blockSize, blockSize - 1);
	int ans2 = GetLcpInBlock(lcpTable, br, 0, r % blockSize);
	int ans = MinDepth(dfs, ans1, ans2);
    
	if (bl < br - 1) 
    {        
		int pw2 = lcpTable->precalcLog2[br - bl - 1];
		int ans3 = lcpTable->sparseTable[bl + 1][pw2];
		int ans4 = lcpTable->sparseTable[br - (1 << pw2)][pw2];
		ans = MinDepth(dfs, ans, MinDepth(dfs, ans3, ans4));
	}    
	return dfs->a[ans];
}

void FreeLcpTable(LcpTable *lcpTable)
{
    debug("free LcpTable: started");
    
    FreeEulerTour(lcpTable->eulerTour);
    
    MemFree(lcpTable->suffixToIndex);
    MemFree(lcpTable->sparseTable);
    MemFree(lcpTable->sparseTableData);
    MemFree(lcpTable->blocksHash);
    MemFree(lcpTable->rmqTable);
    MemFree(lcpTable->rmq2dTables);
    MemFree(lcpTable->rmqTableData);
    MemFree(lcpTable->precalcLog2);
    MemFree(lcpTable);
    
    debug("free LcpTable: finished");
}

// const int MAXN = 100*1000;
// const int MAXLIST = MAXN * 2;
// const int LOG_MAXLIST = 18;
// const int SQRT_MAXLIST = 447;
// const int MAXBLOCKS = MAXLIST / ((LOG_MAXLIST+1)/2) + 1;

// vector<int> g[MAXN];
// int h[MAXN]; // vertex height
// vector<int> a; // dfs list
// int a_pos[MAXN]; // positions in dfs list
// int block; // block size = 0.5 log A.size()
// int bt[MAXBLOCKS][LOG_MAXLIST+1]; // sparse table on blocks (relative minimum positions in blocks)
// int bhash[MAXBLOCKS]; // block hashes
// int brmq[SQRT_MAXLIST][LOG_MAXLIST/2][LOG_MAXLIST/2]; // rmq inside each block, indexed by block hash
// int log2[2*MAXN]; // precalced logarithms (floored values)

