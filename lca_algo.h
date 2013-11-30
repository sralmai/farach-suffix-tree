#pragma once
#include "helpers.h"

typedef struct _lcaTable
{
    DynamicArray *dfsDepths, *dfsToNode;
    
	int blockSize, blocks;
    int **sparseTable, *sparseTableData;
    int *blocksHash;
    int ***rmqTable, **rmq2dTables, *rmqTableData;
    int *precalcLog2;
} LcaTable;

// helpers for multidimensional arrays
int *allocate2D(int ***arr, int n, int m);
int *allocate3D(int ****arr, int ***arr2d, int n, int m, int k);
// -----------------------------------

// compares two indices in a
int MinDepth(DynamicArray *dfsDepths, int i, int j);

LcaTable *CreateLcaTable(DynamicArray *dfsDepths, DynamicArray *dfsToNode);
// answers RMQ in block #bl [l;r] in O(1)
int GetLcaInBlock(LcaTable *lcaTable, int bl, int l, int r);
// answers LCA in O(1)
int GetLca(LcaTable *lcaTable, int l, int r) ;
void FreeLcaTable(LcaTable *lcaTable);

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

