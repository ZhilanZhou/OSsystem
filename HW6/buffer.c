//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

#include "buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h> 
#define ERROR -1

/**
 *  Create an memory mapped file(address is determined by kernel) and return its pointer.
 */
static char* createMMAP(size_t size){
  //These are the neccessary arguments for mmap. See man mmap.
  int protections = PROT_READ|PROT_WRITE; //can read and write
  int flags = MAP_SHARED|MAP_ANONYMOUS; //shared b/w procs & not mapped to a file
  int fd = -1; //We could make it map to a file as well but here it is not needed.
  off_t offset = 0;
    
  //Create memory map
  char* addr =  mmap(NULL, size, protections, flags, fd, offset);
    
  if (( void *) ERROR == addr){ //on an error mmap returns void* -1.
	perror("error with mmap");
	exit(EXIT_FAILURE);
  }
  return addr;
}

/**
 *  Delete the memory mapped file.
 */
static void deleteMMAP(char* addr, size_t size){
	//This deletes the memory map at given address. see man mmap
	if (ERROR == munmap(addr, size)){
		perror("error deleting mmap");
		exit(EXIT_FAILURE);
	}
}

/*
 * Create a new bounded buffer and initialize it.
 */
boundedBuffer *createBuffer(int size, char* semFull, char* semEmpty) { 
	if (size == 0) {
		return NULL;
	}

	boundedBuffer *b = (boundedBuffer*) malloc(sizeof(boundedBuffer));
	b->fullBuffers = semFull;
	b->emptyBuffers = semEmpty;
	
	b->content = createMMAP(size * sizeof(char)); // create memory mapped file for the buffer queue

	b->nextIn = b->nextOut = 0;
	b->size = size;

	return b;
}

/*
 * Delete the corresponding mmap files when delete buffers.
 */
void deleteBuffer(boundedBuffer *b) { 
	//assert(buffer != NULL);
	deleteMMAP(b->content, b->size * sizeof(char)); //delete the mmap file
}

/*
 * Put one char in the buffer content and change the value of two semaphores correspondingly.
 */
void deposit(boundedBuffer *b, char c) {

	sem_t* semFull = sem_open(b->fullBuffers, O_CREAT, S_IREAD | S_IWRITE, 0);
	sem_t* semEmpty = sem_open(b->emptyBuffers, O_CREAT, S_IREAD | S_IWRITE, b->size);

	sem_wait(semEmpty); //condition synchronization
	//assert(b->emptyBuffers >= 0);
	b->content[b->nextIn] = c;
	b->nextIn = (b->nextIn + 1) % b->size;
	
	sem_post(semFull);

	sem_close(semFull);
	sem_close(semEmpty);
}

/*
 * Read one char in the buffer content and change the value of two semaphores correspondingly.
 */
char remoove(boundedBuffer *b) {
	char c;

	sem_t* semFull = sem_open(b->fullBuffers, O_CREAT, S_IREAD | S_IWRITE, 0);
	sem_t* semEmpty = sem_open(b->emptyBuffers, O_CREAT, S_IREAD | S_IWRITE, b->size);

	sem_wait(semFull); //condition synchronization
	//assert(b->fullBuffers >= 0);
	c = b->content[b->nextOut];
	b->nextOut = (b->nextOut + 1) % b->size;
	
	sem_post(semEmpty);

	sem_close(semFull);
	sem_close(semEmpty);

	return c;
}