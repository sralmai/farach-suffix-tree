#pragma once
#include "farach_suftree.h"

typedef struct _eulerTour
{
    int n;
    DynamicArray *dfs;
    int *appearance;
} EulerTour;

typedef struct _eulerTour
{
    DynamicArray *dfs;
    DynamicArray *appearance;
} EulerTour;

EulerTour *GetEulerTour(SuffixTree *tree);
void FreeEulerTour(EulerTour *eulerTour);

typedef struct _lcpTable
{
    SuffixTree *suffixTree;
    EulerTour *eulerTour;
    
    int *suffixToIndex;
    
	int blockSize, blocks;
    int **sparseTable, *sparseTableData;
    int *blocksHash;
    int ***rmqTable, **rmq2dTables, *rmqTableData;
    int *precalcLog2;
} LcpTable;

LcpTable *CreateLcpTable(SuffixTree *tree);
void BuildLcpTable(LcpTable *lcpTable);
int MinDepth(DynamicArray *dfs, int i, int j);
int GetLcpInBlock(LcpTable *lcpTable, int bl, int l, int r);
int GetLcp(LcpTable *lcpTable, int v1, int v2);
void FreeLcpTable(LcpTable *lcpTable);