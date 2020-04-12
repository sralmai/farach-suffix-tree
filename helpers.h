#pragma once

/* -------------- debugPrintf Methods ------------- */
int DEBUG, DYNAMIC_ARRAY_MIN_CAPACITY;
#define debugPrintf(fmt, ...) \
    do { if (DEBUG) { fprintf(stdout, fmt, __VA_ARGS__); fflush(stdout); }} while (0)
    
void debugPrint(const char *s);
void debugArr(int *a, int n);
void debugInt(int x);
void debugIntVar(int x, const char *s);

/* -------------- Helpers ------------------ */
char *ConcatStrings(const char *s1, const char *s2);
void MemFree(void *a);
int Log2(int x);
void Swap(int *x, int *y);


/* ------------ Resizeable Array ----------- */
typedef struct _dynamicArray
{
    int count, capacity;
    int *a;
} DynamicArray;

DynamicArray *CreateDynamicArray(int capacity);
void FreeDynamicArray(DynamicArray *arr);

int *LastInDynamicArray(DynamicArray *arr);
void PushToDynamicArray(DynamicArray *arr, int x);
int PopFromDynamicArray(DynamicArray *arr);


/* ------- Resizeable Circled Queue -------- */
typedef struct _dynamicQueue
{
    int top, bottom, count, capacity;
    int *a;
} DynamicQueue;

DynamicQueue *CreateDynamicQueue(int capacity);
void FreeDynamicQueue(DynamicQueue *queue);

void PushToDynamicQueue(DynamicQueue *queue, int x);
int PopFromDynamicQueue(DynamicQueue *queue);
int PeekToDynamicQueue(DynamicQueue *queue);
