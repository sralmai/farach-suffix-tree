#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lca_algo.h"


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
inline int MinDepth(DynamicArray *dfsDepths, int i, int j) 
{
	return dfsDepths->a[i] <= dfsDepths->a[j] ? i : j;
}

LcaTable *CreateLcaTable(DynamicArray *dfsDepths, DynamicArray *dfsToNode)
{
    debug("create LcaTable: started");
                
    /* 
        n = dfsDepths array length
        blockSize = size of block  (Log2(n) upper bounded)
        blocks = blocks count
        sparseTable = array[blocks][Log2(n) + 1] initialized with -1
        blocksHash = array[blocks] initialized with 0
        rmqTable = array[sqrtN][blockSize][blockSize] initialized with -1
    */
    
    LcaTable *lcaTable = calloc(1, sizeof *lcaTable);
	int n = dfsDepths->count, log2n = Log2(n);
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
		if (sparseTable[bl][0] == -1 || MinDepth(dfsDepths, i, sparseTable[bl][0]) == i)
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
				sparseTable[i][j] = MinDepth(dfsDepths, sparseTable[i][j - 1], sparseTable[ni][j - 1]);
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
        
		if (j > 0 && (i >= n || MinDepth(dfsDepths, i - 1, i) == i - 1))
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
						MinDepth(dfsDepths, i * blockSize + rmqTable[id][l][r], i * blockSize + r) - i * blockSize;
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
        
    lcaTable->dfsDepths = dfsDepths;
    lcaTable->dfsToNode = dfsToNode;
	lcaTable->blockSize = blockSize;
    lcaTable->blocks = blocks;
    lcaTable->sparseTable = sparseTable;
    lcaTable->sparseTableData = sparseTableData;
    lcaTable->blocksHash = blocksHash;
    lcaTable->rmqTable = rmqTable;
    lcaTable->rmq2dTables = rmq2dTables;
    lcaTable->rmqTableData = rmqTableData;
    lcaTable->precalcLog2 = precalcLog2;
    
    debug("create LcaTable: finished");
    
    return lcaTable;
}

// answers RMQ in block #bl [l;r] in O(1)
inline int GetLcaInBlock(LcaTable *lcaTable, int bl, int l, int r) 
{
	return lcaTable->rmqTable[lcaTable->blocksHash[bl]][l][r] + bl * lcaTable->blockSize;
}

// answers LCA in O(1)
int GetLca(LcaTable *lcaTable, int l, int r) 
{    
	int 
        blockSize = lcaTable->blockSize,
        bl = l / blockSize, 
        br = r / blockSize;
    
	if (bl == br)
		return lcaTable->dfsToNode->a[GetLcaInBlock(lcaTable, bl, l % blockSize, r % blockSize)];
        
	int ans1 = GetLcaInBlock(lcaTable, bl, l % blockSize, blockSize - 1);
	int ans2 = GetLcaInBlock(lcaTable, br, 0, r % blockSize);
	int ans = MinDepth(lcaTable->dfsDepths, ans1, ans2);
    
	if (bl < br - 1) 
    {        
		int pw2 = lcaTable->precalcLog2[br - bl - 1];
		int ans3 = lcaTable->sparseTable[bl + 1][pw2];
		int ans4 = lcaTable->sparseTable[br - (1 << pw2)][pw2];
		ans = MinDepth(
            lcaTable->dfsDepths, 
            ans, 
            MinDepth(lcaTable->dfsDepths, ans3, ans4)
        );
	}    
	return lcaTable->dfsToNode->a[ans];
}

void FreeLcaTable(LcaTable *lcaTable)
{
    debug("free LcaTable: started");
    
    MemFree(lcaTable->sparseTable);
    MemFree(lcaTable->sparseTableData);
    MemFree(lcaTable->blocksHash);
    MemFree(lcaTable->rmqTable);
    MemFree(lcaTable->rmq2dTables);
    MemFree(lcaTable->rmqTableData);
    MemFree(lcaTable->precalcLog2);
    MemFree(lcaTable);
    
    debug("free LcaTable: finished");
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

