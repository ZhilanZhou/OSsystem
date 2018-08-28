//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define READ 0
#define WRITE 1
#define LINE_LENGTH 80

int pipe1[2];
int pipe2[2];
int pipe3[2];

/*
 * Read from stdin and write to pipe1 for the next process carriageToSpace.
 * When get EOF, write an EOF in the pipe and exit.
 */
void inputProcessing(pid_t child_pid) {
	close(pipe1[READ]);
	close(pipe2[READ]);
	close(pipe2[WRITE]);
	close(pipe3[READ]);
	close(pipe3[WRITE]);
	int child_status;

	char c;

	while((c = getchar()) != EOF) {
		if (write(pipe1[WRITE], &c, 1) == -1) {
			perror("inputProcessing Error\n");
			break;
		}
	}

	c = EOF;
	if (write(pipe1[WRITE], &c, 1) == -1) {
		perror("inputProcessing Error\n");
	}

	close(pipe1[WRITE]);
	waitpid(child_pid, &child_status, 0);
	exit(0);
}

/*
 * Read from pipe1, change '\n' to space and write to pipe2 for the next process asterisksToCaret.
 * When get EOF, write an EOF in the pipe and exit.
 */
void carriageToSpace(pid_t child_pid) {
	close(pipe1[WRITE]);
	close(pipe2[READ]);
	close(pipe3[READ]);
	close(pipe3[WRITE]);
	int child_status;

	char c;

	while(read(pipe1[READ], &c, 1)) {
		if(c == '\n') {
			c = ' ';
		}
		if (write(pipe2[WRITE], &c, 1) == -1) {
			perror("carriageToSpace Error\n");
			break;
		}
	}

	c = EOF;
	if (write(pipe2[WRITE], &c, 1) == -1) {
		perror("carriageToSpace Error\n");
	}

	close(pipe1[READ]);
	close(pipe2[WRITE]);
	waitpid(child_pid, &child_status, 0);
	exit(0);
}

/*
 * Read from pipe2, change '**' to '^' and write to pipe3 for the next process outputProcessing.
 * When get EOF, write an EOF in the pipe and exit.
 */
void asterisksToCaret(pid_t child_pid) {
	close(pipe1[READ]);
	close(pipe1[WRITE]);
	close(pipe2[WRITE]);
	close(pipe3[READ]);
	int child_status;

	char c, nextc;

	while(read(pipe2[READ], &c, 1)) {
		if(c == '*') {
			if(read(pipe2[READ], &nextc, 1)){
				if(nextc == '*') {
					c = '^';
					if (write(pipe3[WRITE], &c, 1) == -1) {
						perror("asterisksToCaret Error\n");
					}
				} else {
					if (write(pipe3[WRITE], &c, 1) == -1) {
						perror("asterisksToCaret Error\n");
					}
					if (write(pipe3[WRITE], &nextc, 1) == -1) {
						perror("asterisksToCaret Error\n");
					}
				}
			} else {
				if (write(pipe3[WRITE], &c, 1) == -1) {
					perror("asterisksToCaret Error\n");
				}
			}
		} else {
			if (write(pipe3[WRITE], &c, 1) == -1) {
				perror("asterisksToCaret Error\n");
			}
		}
	}
	c = EOF;
	if (write(pipe3[WRITE], &c, 1) == -1) {
		perror("asterisksToCaret Error\n");
	}

	close(pipe2[READ]);
	close(pipe3[WRITE]);
	waitpid(child_pid, &child_status, 0);
	exit(0);
}

/*
 * Read from pipe3, print out lines.
 * When get EOF, exit directly.
 */
void outputProcessing() {
	close(pipe1[READ]);
	close(pipe1[WRITE]);
	close(pipe2[READ]);
	close(pipe2[WRITE]);
	close(pipe3[WRITE]);

	int count = 0;
	char line[LINE_LENGTH + 1];
	char c;
	line[LINE_LENGTH] = '\0';

	while(read(pipe3[READ], &c, 1)) {
		line[count] = c;
		count++;
		if(count == LINE_LENGTH) {
			printf("%s\n", line);
			fflush(stdout);
			count = 0;
		}
	}
	close(pipe3[READ]);	
	exit(0);
}

int main () {
	pid_t pid1, pid2, pid3, pid4; //pid variables for confirming the current process
	
	if (pipe(pipe1) == -1) {
		perror("pipe 1 Error\n");
		exit(EXIT_FAILURE);
	}
	if (pipe(pipe2) == -1) {
		perror("pipe 2 Error\n");
		exit(EXIT_FAILURE);
	}
	if (pipe(pipe3) == -1) {
		perror("pipe 3 Error\n");
		exit(EXIT_FAILURE);
	}

	pid1 = getpid();
	
	pid2 = fork();
	if (pid2 == -1) {
		perror("fork error in carriageToSpace\n");
		exit(-1);
	}else if (pid2) { 
		//parent
		inputProcessing(pid2);
	}else { 
		//new child
		pid3 = fork();
		if (pid3 == -1) {
			perror("fork error in asterisksToCaret\n");
			exit(-1);
		}else if (pid3) {
			carriageToSpace(pid3);
		}else {
			//new child
			pid4 = fork();
			if (pid4 == -1) {
				perror("fork error in outputProcessing\n");
				exit(-1);
			}else if (pid4) {
				asterisksToCaret(pid4);
			}else {
				//final process
				outputProcessing();
			}
		}		
	}
	return 0;
}