#pragma once

void debug(const char *s);
void debugArr(int *a, int n);

char *ConcatStrings(const char *s1, const char *s2);
void MemFree(void *a);
int Log2(int x);

/* ----------- Resizeable Array -------- */
typedef struct _dynamicArray
{
    int count, capacity;
    int *a;
} DynamicArray;

DynamicArray *CreateDynamicArray(int capacity);
void AllocateNextIndexInDynamicArray(DynamicArray *arr);
int *TopInDynamicArray(DynamicArray *arr);
void PushToDynamicArray(DynamicArray *arr);
void FreeDynamicArray(DynamicArray *arr);
