//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

#include "buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 * Create a new bounded buffer and initialize it.
 */
boundedBuffer *createBuffer(int size) { 
	if (size == 0) {
		return NULL;
	}

	boundedBuffer *b = (boundedBuffer*) malloc(sizeof(boundedBuffer));
	b->content = (char*)malloc(size * sizeof(char)); // allocate memory for the buffer queue

	createSem(&b->emptyBuffers, size);
	createSem(&b->fullBuffers, 0);
	b->nextIn = b->nextOut = 0;
	b->size = size;

	return b;
}

/*
 * Delete the buffer and free the memory.
 */
void deleteBuffer(boundedBuffer *b) { 
	free(b->content); //when delete, free the space
	b->content = NULL;
}

/*
 * Put one char in the buffer content and change the value of two semaphores correspondingly.
 */
void deposit(boundedBuffer *b, char c) {
	down(&b->emptyBuffers); //condition synchronization
	//assert(b->emptyBuffers >= 0);
	b->content[b->nextIn] = c;
	b->nextIn = (b->nextIn + 1) % b->size;
	
	up(&b->fullBuffers);
}

/*
 * Read one char in the buffer content and change the value of two semaphores correspondingly.
 */
char remoove(boundedBuffer *b) {
	char c;

	down(&b->fullBuffers); //condition synchronization
	//assert(b->fullBuffers >= 0);
	c = b->content[b->nextOut];
	b->nextOut = (b->nextOut + 1) % b->size;
	
	up(&b->emptyBuffers);

	return c;
}