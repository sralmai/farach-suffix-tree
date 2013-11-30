#pragma once

/* -------------- Debug Methods ------------- */
void debug(const char *s);
void debugArr(int *a, int n);


/* -------------- Helpers ------------------ */
char *ConcatStrings(const char *s1, const char *s2);
void MemFree(void *a);
int Log2(int x);
void Swap(void *x, void *y);


/* ------------ Resizeable Array ----------- */
typedef struct _dynamicArray
{
    int count, capacity;
    int *a;
} DynamicArray;

DynamicArray *CreateDynamicArray(int capacity);
void AllocateNextIndexInDynamicArray(DynamicArray *arr);
int *LastInDynamicArray(DynamicArray *arr);
void PushToDynamicArray(DynamicArray *arr, int x);
int PopFromDynamicArray(DynamicArray *arr);
void FreeDynamicArray(DynamicArray *arr);