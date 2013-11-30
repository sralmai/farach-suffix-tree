#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

/* -------------- Debug Methods ------------- */

void debug(const char *s)
{
    printf("==> %s\n", s);
    fflush(stdout);
}

void debugArr(int *a, int n)
{
    printf("[");
    if (n > 0)
    {
        printf("%d", a[0]);
        for (int i = 1; i < n; i++)
            printf(", %d", a[i]);
    }
    printf("]\n");
    fflush(stdout);
}

/* -------------- Helpers ------------------ */

char *ConcatStrings(const char *s1, const char *s2)
{
    char *s = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(s, s1);
    strcpy(s + strlen(s1), s2);
    
    return s;
}

inline void MemFree(void *a)
{
    if (a) free(a);
}

int Log2(int x) 
{
	int res = 0;
	while (1 << res < x)  
        res++;
    
	return res;
}

void Swap(void *x, void *y)
{
    void *t = x;
    x = y;
    y = t;
}

/* ------------ Resizeable Array ----------- */

DynamicArray *CreateDynamicArray(int capacity)
{
    DynamicArray *res = calloc(1, sizeof *res);
    res->a = malloc(capacity * sizeof *res->a);
    res->capacity = capacity;
    res->count = 0;
    
    return res;
}
void AllocateNextIndexInDynamicArray(DynamicArray *arr)
{
    if (arr->count == arr->capacity)
    {
        // double the capacity
        int *t = realloc(arr->a, 2 * arr->capacity * sizeof *t);
        if (t)
        {
            arr->a = t;
            arr->capacity = arr->capacity << 1;
        }
    }
    arr->count ++;
}
inline int *LastInDynamicArray(DynamicArray *arr)
{
    return (arr->a + arr->count - 1);
}
void PushToDynamicArray(DynamicArray *arr, int x)
{
    AllocateNextIndexInDynamicArray(arr);
    *LastInDynamicArray(arr) = x;
}
int PopFromDynamicArray(DynamicArray *arr)
{
    return arr->a[--(arr->count)];
}

void FreeDynamicArray(DynamicArray *arr)
{
    MemFree(arr->a);
    MemFree(arr);
}