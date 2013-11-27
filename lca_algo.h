#pragma once
#include "farach_suftree.h"

typedef struct _lcaTable
{
    DynamicArray *dfsDepths;
    
	int blockSize, blocks;
    int **sparseTable, *sparseTableData;
    int *blocksHash;
    int ***rmqTable, **rmq2dTables, *rmqTableData;
    int *precalcLog2;
} LcaTable;
