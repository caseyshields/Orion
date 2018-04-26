/* Heap.c */

#include "Heap.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifndef PRINT
#define PRINT printf("(%s:%i)\n", __FILE__, __LINE__);
#endif

void Heap_rise(int root, void *obj, Heap *heap);
void Heap_sink(int root, void *obj, Heap *heap);

Heap * Heap_create(int allocated, int(*compare)(), int discard) {
	Heap *heap;

	assert(allocated);
	assert(compare);

	heap = malloc(sizeof(Heap));
	if(!heap) return NULL;	//assert(heap);

	heap->allocated = allocated;
	heap->compare = compare;
	heap->discard = discard;
	heap->size = 0;
	heap->data = calloc(allocated, sizeof(void*));
	if(!(heap->data)) {
		free(heap);
		return NULL;
	}	//assert(heap->data);
	return heap;
}

void Heap_push(void *obj, Heap *heap) {
	assert( obj );
	assert( heap );
	assert( heap->data );
	assert( heap->compare );
	if(heap->size == heap->allocated) {
		assert(0); // TODO
		if(heap->discard);//insert and drop excess
		else; // double size of array
	} else {
		Heap_rise(heap->size, obj, heap);
		heap->size++;
	}
}

void Heap_rise(int root, void *obj, Heap * heap) {
	void *v = obj;
	int parent = 0;
	while(root) {
		//parent = (root - (root%2)) / 2;
		parent = root/2 - ((root+1)%2); // branchless
		v = heap->data[parent];
		if(heap->compare(obj, v)>0) {
			heap->data[root] = v;
			root = parent;
		} else { break; }
	}
	heap->data[root] = obj;
}

void * Heap_peek(Heap *heap) {
	assert(heap && heap->data && heap->size>0);
	return heap->data[0];
}

void * Heap_pull(Heap *heap) {
	void *t;
	assert( heap );
	assert( heap->data );
	assert( heap->compare );
	assert( heap->size>0 );
	t = heap->data[0];
	heap->size--;
	heap->data[0] = heap->data[heap->size];
	heap->data[heap->size] = t;
	Heap_sink(0, heap->data[0], heap);
	return t;//heap->data[heap->size];
}

void Heap_sink(int root, void *obj, Heap *heap) {
	int c1, c2, cmax;
	void *v1, *v2, *vmax;
	do {
		c1 = 2*root+1;
		c2 = c1+1;
		if(c2>=heap->size) {
			if(c1>=heap->size)
				break;
			else cmax = c1;
		} else {
			v1 = heap->data[c1];
			v2 = heap->data[c2];
			if( heap->compare(v1,v2)>0 )
				cmax = c1;
			else cmax = c2;
		}
		vmax = heap->data[cmax];
		if( heap->compare(vmax, obj)>0 ) {
			heap->data[root] = vmax;
			root = cmax;
		} else break;
	} while(1);
	heap->data[root] = obj;
}

void Heap_destroy(Heap *heap) {
	assert(heap && heap->data);
	free(heap->data);
	free(heap);
}
