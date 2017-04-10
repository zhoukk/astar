/**
 * author: zhoukk
 * link: https://github.com/zhoukk/astar/blob/master/heap.h
 *
 * MinHeap or MaxHeap
 *
 *
 */

#ifndef _heap_h_
#define _heap_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HEAP_API
#define HEAP_API extern
#endif // HEAP_API

/** compare function for order heap */
typedef int (* heap_compare)(void *va, void *vb);

struct heap;

/** Create a heap array with default cap n, and compare function f. */
HEAP_API struct heap *heap_new(int n, heap_compare f);

/** Release the heap array. */
HEAP_API void heap_free(struct heap *heap);

/** Clear heap array, just make size == 0. */
HEAP_API void heap_clear(struct heap *heap);

/** Push an object v to heap array. */
HEAP_API int heap_push(struct heap *heap, void *v);

/** Pop object in top of array. */
HEAP_API void *heap_pop(struct heap *heap);

/** Return object in top of array. */
HEAP_API void *heap_top(struct heap *heap);

/** Update object v in heap array orderd. */
HEAP_API void heap_update(struct heap *heap, void *v);

/** Remove the object v in heap array. */
HEAP_API void *heap_remove(struct heap *heap, void *v);

/** Check exist of object v in heap array. */
HEAP_API int heap_exist(struct heap *heap, void *v);

#ifdef __cplusplus
}
#endif

#endif // _heap_h_


#ifdef HEAP_IMPLEMENTATION

/**
 * Implement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct heap {
    int cur;
    int cap;
    heap_compare f;
    void **array;
};

HEAP_API struct heap *
heap_new(int n, heap_compare f) {
    struct heap *heap = (struct heap *)malloc(sizeof *heap);
    if (!heap) return 0;
    heap->array = (void **)calloc(n, sizeof(void *));
    heap->cur = 0;
    heap->cap = n;
    heap->f = f;
    return heap;
}

HEAP_API void
heap_free(struct heap *heap) {
    free(heap->array);
    free(heap);
}

HEAP_API void
heap_clear(struct heap *heap) {
    heap->cur = 0;
}

static void
heap_adjust_up(struct heap *heap, int i) {
    int p = (i+1)/2-1;
    void *v = heap->array[i];
    while (i > 0 && heap->f(v, heap->array[p])) {
        heap->array[i] = heap->array[p];
        i = p;
        p = (i+1)/2-1;
    }
    heap->array[i] = v;
}

static void
heap_adjust_down(struct heap *heap, int i) {
    void *v = heap->array[i];
    for (;;) {
        int l = (i+1)*2-1;
        int r = l+1;
        if (l < heap->cur-1) {
            int k = r;
            if (r == heap->cur-1 || heap->f(heap->array[l], heap->array[r])) {
                k = l;
            }
            if (heap->f(v, heap->array[k])) {
                break;
            }
            heap->array[i] = heap->array[k];
            i = k;
        } else break;
    }
    heap->array[i] = v;
}

HEAP_API int
heap_push(struct heap *heap, void *v) {
    if (heap->cur >= heap->cap) {
        void **array = (void **)calloc(heap->cap * 2, sizeof(void *));
        if (!array) return -1;
        memcpy(array, heap->array, heap->cap * sizeof(void *));
        free(heap->array);
        heap->array = array;
        heap->cap *= 2;
    }

    heap->array[heap->cur] = v;
    heap_adjust_up(heap, heap->cur);
    heap->cur ++;
    return 0;
}

HEAP_API void *
heap_pop(struct heap *heap) {
    if (heap->cur <= 0) return 0;
    void *v = heap->array[0];
    heap->array[0] = heap->array[heap->cur-1];
    heap_adjust_down(heap, 0);
    heap->cur --;
    return v;
}

HEAP_API void *
heap_top(struct heap *heap) {
    if (heap->cur <= 0) return 0;
    return heap->array[0];
}

HEAP_API void
heap_update(struct heap *heap, void *v) {
    int i;
    for (i = 0; i < heap->cur; i++) {
        if (heap->array[i] == v) {
            heap_adjust_up(heap, i);
            heap_adjust_down(heap, i);
            return;
        }
    }
}

HEAP_API void *
heap_remove(struct heap *heap, void *v) {
    int i;
    for (i = 0; i < heap->cur; i++) {
        if (heap->array[i] == v) {
            heap->array[i] = heap->array[heap->cur-1];
            heap_adjust_down(heap, i);
            heap->cur --;
            return v;
        }
    }
    return 0;
}

HEAP_API int
heap_exist(struct heap *heap, void *v) {
    int i;
    for (i = 0; i < heap->cur; i++) {
        if (heap->array[i] == v) {
            return 1;
        }
    }
    return 0;
}

#endif // HEAP_IMPLEMENTATION
