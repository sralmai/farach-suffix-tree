#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

/* -------------- DEBUG Methods ------------- */    
inline void debugPrint(const char *s)
{
    debugPrintf(s, NULL);
}
    
void debugArr(int *a, int n)
{
    debugPrint("[");
    if (n > 0)
    {
        debugPrintf("%d", a[0]);
        for (int i = 1; i < n; i++)
        {
            debugPrintf(", %d", a[i]);
        }
    }
    debugPrint("]\n");
}

inline void debugInt(int x)
{
    debugPrintf("%d\n", x);
}

inline void debugIntVar(int x, const char *s)
{
    debugPrintf("%s = %d\n", s, x);
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

void Swap(int *x, int *y)
{
    int t = *x;
    *x = *y;
    *y = t;
}

/* ------------ Resizeable Array ----------- */

DynamicArray *CreateDynamicArray(int capacity)
{
    DynamicArray *arr = calloc(1, sizeof *arr);
    arr->a = malloc(capacity * sizeof *arr->a);
    arr->capacity = capacity;
    arr->count = 0;
    
    return arr;
}
void FreeDynamicArray(DynamicArray *arr)
{
    if (!arr)
        return;
        
    MemFree(arr->a);
    MemFree(arr);
}

inline int *LastInDynamicArray(DynamicArray *arr)
{
    return (arr->a + arr->count - 1);
}
void PushToDynamicArray(DynamicArray *arr, int x)
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
    arr->a[arr->count] = x;
    ++(arr->count);
}
int PopFromDynamicArray(DynamicArray *arr)
{
    return arr->a[--(arr->count)];
}

/* ------- Resizeable Circled Queue -------- */

DynamicQueue *CreateDynamicQueue(int capacity)
{
    DynamicQueue *queue = calloc(1, sizeof *queue);
    queue->a = malloc(capacity * sizeof *queue->a);
    queue->capacity = capacity;
    queue->count = 0;
    queue->top = 0;
    queue->bottom = 0;
    
    return queue;
}

void FreeDynamicQueue(DynamicQueue *queue)
{
    if (!queue)
        return;
        
    MemFree(queue->a);
    MemFree(queue);
}

void PushToDynamicQueue(DynamicQueue *queue, int x)
{
    if (queue->count == queue->capacity)
    {
        // double the capacity
        int *t = realloc(queue->a, 2 * queue->capacity * sizeof *t);
        if (t)
        {
            queue->a = t;
            
            if (queue->top <= queue->bottom)
            {
                if (queue->top < (queue->capacity - queue->bottom))
                {
                    memcpy(queue->a + queue->capacity, queue->a, queue->top * sizeof *queue->a);
                    queue->top += queue->capacity;
                }
                else
                {
                    memcpy(queue->a + queue->bottom + queue->capacity, queue->a + queue->bottom, (queue->capacity - queue->bottom) * sizeof *queue->a);
                    queue->bottom += queue->capacity;
                }
            }
            queue->capacity = queue->capacity << 1;
        }
    }
        
    queue->a[queue->top] = x;
    ++(queue->top);
    ++(queue->count);
    
    if (queue->top == queue->capacity)
        queue->top = 0;
}

int PopFromDynamicQueue(DynamicQueue *queue)
{
    int x = PeekToDynamicQueue(queue);
    ++(queue->bottom);
    --(queue->count);
    
    if (queue->bottom == queue->capacity)
        queue->bottom = 0;
    
    return x;
}

inline int PeekToDynamicQueue(DynamicQueue *queue)
{
    return queue->a[queue->bottom];
}