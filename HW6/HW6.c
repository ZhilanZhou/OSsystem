//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

/*
 * This program will read from stdin, change '\n' to space, change '**' to '^', and finally write to stdout
 * with lines of 80 characters.
 */

#include "buffer.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <semaphore.h> 
#include <sys/mman.h>

#define BUFFER_SIZE 50
#define LINE_LENGTH 80
#define SF1 "/semFull1"
#define SF2 "/semFull2"
#define SF3 "/semFull3"
#define SE1 "/semEmpty1"
#define SE2 "/semEmpty2"
#define SE3 "/semEmpty3"
#define ERROR -1

typedef struct {
	boundedBuffer *inputBuffer, *outputBuffer;
} processInit;

/*
 * Read from stdin and write to output buffer for the next process carriageToSpace.
 * When get EOF, write an EOF in the buffer and exit.
 */
void inputProcessing(void *args) {
	boundedBuffer * out = ((processInit *)args)->outputBuffer;
	char c;

	while((c = getchar()) != EOF) {
		deposit(out, c);
	}
	deposit(out, EOF);
	_Exit(0);
}

/*
 * Read from input buffer, change '\n' to space and write to output buffer for the next process asterisksToCaret.
 * When get EOF, write an EOF in the buffer and exit.
 */
void carriageToSpace(void *args) {
	boundedBuffer *in = ((processInit *)args)->inputBuffer;
	boundedBuffer *out = ((processInit *)args)->outputBuffer;
	char c;

	while((c = remoove(in)) != EOF) {
		if(c == '\n') {
			c = ' ';
		}
		deposit(out, c);
	}
	deposit(out, EOF);
	_Exit(0);
}

/*
 * Read from input buffer, change '**' to '^' and write to output buffer for the next process outputProcessing.
 * When get EOF, write an EOF in the buffer and exit.
 */
void asterisksToCaret(void *args) {
	boundedBuffer *in = ((processInit *)args)->inputBuffer;
	boundedBuffer *out = ((processInit *)args)->outputBuffer;
	char c, nextc;

	while((c = remoove(in)) != EOF) {
		if(c == '*') {
			nextc = remoove(in);
			if(nextc == '*') {
				deposit(out, '^');
			} else if(nextc == EOF) {
				deposit(out, c);
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
	_Exit(0);
}

/*
 * Read from input buffer, print out lines.
 * When get EOF, exit directly.
 */
void outputProcessing(void *args) {
	boundedBuffer *in = ((processInit *)args)->inputBuffer;
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
	_Exit(0);
}

/* 
 * This function takes a pointer to a function as an argument
 * and the functions argument. It then returns the forked child's pid.
 */
pid_t forkChild(void (*function)(void *), void *args) {
	pid_t childpid;
	switch (childpid = fork()) {
		case ERROR:
		perror("Fork error");
		exit(EXIT_FAILURE);
		case 0:
		(*function)(args);
		default:
		return childpid;
	}
}

/* 
 * This function takes a child's pid and the total number of child processes as arguments.
 * It will do nothing until a child process exit with error, in which case it will kill all the children processes.
 */
void waitForChildren(pid_t* childpids, int n) {
	int status;
	int i; 
	while (ERROR < wait(&status)) { //Here the parent waits on any child.
    	if (!WIFEXITED(status)) {
      		for (i = 0; i < n; ++i) { //If the termination err, kill all children.
      			kill(childpids[i], SIGKILL);
      		}
      	break;
  		}
	}
}

int main() {

	boundedBuffer *buf1 = createBuffer(BUFFER_SIZE, SF1, SE1);
	boundedBuffer *buf2 = createBuffer(BUFFER_SIZE, SF2, SE2);
	boundedBuffer *buf3 = createBuffer(BUFFER_SIZE, SF3, SE3); //buffers for sharing between threads

	processInit inputProcessingInit;
	processInit carriageToSpaceInit;
	processInit asterisksToCaretInit;  
	processInit outputProcessingInit; //args for starting threads

	inputProcessingInit.inputBuffer = NULL;
	inputProcessingInit.outputBuffer = buf1;

	carriageToSpaceInit.inputBuffer = buf1;
	carriageToSpaceInit.outputBuffer = buf2;

	asterisksToCaretInit.inputBuffer = buf2;
	asterisksToCaretInit.outputBuffer = buf3;

	outputProcessingInit.inputBuffer = buf3;
	outputProcessingInit.outputBuffer = NULL; //bound buffers to processInit

	pid_t childpids[4];

	childpids[0] = forkChild(inputProcessing, &inputProcessingInit);
	childpids[1] = forkChild(carriageToSpace, &carriageToSpaceInit);
 	childpids[2] = forkChild(asterisksToCaret, &asterisksToCaretInit);
 	childpids[3] = forkChild(outputProcessing, &outputProcessingInit); //fork 4 childs to process

 	waitForChildren(childpids, 4);

 	deleteBuffer(buf1);
 	deleteBuffer(buf2);
 	deleteBuffer(buf3); //clear buffers

 	//clear semaphores
 	if(sem_unlink(SF1) == ERROR || sem_unlink(SE1) == ERROR ||
    	sem_unlink(SF2) == ERROR || sem_unlink(SE2) == ERROR ||
    	sem_unlink(SF3) == ERROR || sem_unlink(SE3) == ERROR) {
		perror("error while unlinking semaphore");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
    return 0;
}