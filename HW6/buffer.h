//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

#include <fcntl.h>
#include <semaphore.h>
#include <stddef.h>
#include <sys/mman.h>

typedef struct {
	char *content;
	int size;
	char *fullBuffers, *emptyBuffers;
	int nextIn, nextOut;
} boundedBuffer;

extern boundedBuffer *createBuffer (int size, char* semFull, char* semEmpty);
extern void deleteBuffer (boundedBuffer *b);
extern void deposit (boundedBuffer *b, char c);
extern char remoove (boundedBuffer *b);