/* Heap.h
 * Full binary tree heap, implemented in array.
 * Guaranteed O(log n) push and pull.
 */

#ifndef HEAP_H_
#define HEAP_H_

typedef struct {
	int allocated;
	int size;
	void **data;
	int (*compare)();
	int discard;
} Heap;

Heap * Heap_create(int allocated, int (*compare)(), int discard);
void   Heap_push(void *obj, Heap *heap);
void * Heap_pull(Heap *heap);
void * Heap_peek(Heap *heap);
void   Heap_destroy(Heap *heap);

#endif /* HEAP_H_ */
