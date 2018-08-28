//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

#include "buffer.h"
#include "st.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BUFFER_SIZE 30
#define LINE_LENGTH 80

typedef struct {
	boundedBuffer *inputBuffer, *outputBuffer;
} threadInit;

/*
 * Read from stdin and write to output buffer for the next process carriageToSpace.
 * When get EOF, write an EOF in the buffer and exit.
 */
void *inputProcessing(void *args) {
	boundedBuffer * out = ((threadInit *)args)->outputBuffer;
	char c;

	while((c = getchar()) != EOF) {
		deposit(out, c);
	}
	deposit(out, EOF);
	st_thread_exit(NULL);
}

/*
 * Read from input buffer, change '\n' to space and write to output buffer for the next process asterisksToCaret.
 * When get EOF, write an EOF in the buffer and exit.
 */
void *carriageToSpace(void *args) {
	boundedBuffer *in = ((threadInit *)args)->inputBuffer;
	boundedBuffer *out = ((threadInit *)args)->outputBuffer;
	char c;

	while((c = remoove(in)) != EOF) {
		if(c == '\n') {
			c = ' ';
		}
		deposit(out, c);
	}
	deposit(out, EOF);
	st_thread_exit(NULL);
}

/*
 * Read from input buffer, change '**' to '^' and write to output buffer for the next process outputProcessing.
 * When get EOF, write an EOF in the buffer and exit.
 */
void *asterisksToCaret(void *args) {
	boundedBuffer *in = ((threadInit *)args)->inputBuffer;
	boundedBuffer *out = ((threadInit *)args)->outputBuffer;
	char c, nextc;

	while((c = remoove(in)) != EOF) {
		if(c == '*') {
			nextc = remoove(in);
			if(nextc == '*') {
				deposit(out, '^');
			} else if(nextc == EOF) {
				break;
			} else {
				deposit(out, c);
				deposit(out, nextc);
			}
		} else {
			deposit(out, c);
		}
	}
	deposit(out, EOF);
	st_thread_exit(NULL);
}

/*
 * Read from input buffer, print out lines.
 * When get EOF, exit directly.
 */
void *outputProcessing(void *args) {
	boundedBuffer *in = ((threadInit *)args)->inputBuffer;
	int count = 0;
	char line[LINE_LENGTH + 1];
	char c;
	line[LINE_LENGTH] = '\0';

	while((c = remoove(in)) != EOF) {
		line[count] = c;
		count++;
		if(count == LINE_LENGTH) {
			printf("%s\n", line);
			fflush(stdout);
			count = 0;
		}
	}
	st_thread_exit(NULL);
}

int main() {

	if(st_init() < 0) {
		perror("Initialization Error");
		exit(1);
	}

	boundedBuffer *buf1 = createBuffer(BUFFER_SIZE);
	boundedBuffer *buf2 = createBuffer(BUFFER_SIZE);
	boundedBuffer *buf3 = createBuffer(BUFFER_SIZE); //buffers for sharing between threads

	threadInit inputProcessingInit;
	threadInit carriageToSpaceInit;
	threadInit asterisksToCaretInit;
	threadInit outputProcessingInit; //args for starting threads

	inputProcessingInit.inputBuffer = NULL;
	inputProcessingInit.outputBuffer = buf1;

	carriageToSpaceInit.inputBuffer = buf1;
	carriageToSpaceInit.outputBuffer = buf2;

	asterisksToCaretInit.inputBuffer = buf2;
	asterisksToCaretInit.outputBuffer = buf3;

	outputProcessingInit.inputBuffer = buf3;
	outputProcessingInit.outputBuffer = NULL;

	if (st_thread_create(inputProcessing, &inputProcessingInit, 0, 0)==NULL) {
		perror("Thread creation Error \n");
		exit(1);
	}
	
	if (st_thread_create(carriageToSpace, &carriageToSpaceInit, 0, 0)==NULL) {
		perror("Thread creation Error \n");
		exit(1);
	}
	
	if (st_thread_create(asterisksToCaret, &asterisksToCaretInit, 0, 0)==NULL) {
		perror("Thread creation Error \n");
		exit(1);
	}
	
	if (st_thread_create(outputProcessing, &outputProcessingInit, 0, 0)==NULL) {
		perror("Thread creation Error \n");
		exit(1);
	}
	
	st_thread_exit(NULL);
	
    return 0;
}