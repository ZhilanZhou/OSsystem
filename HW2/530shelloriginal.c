//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

//Size of a single command
#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];
pid_t childPid;
//characters of whitespace
const char whiteSpace[]=" \f\n\r\t\v";

void searchAndExec(char* fname, char* argv[]) {
	struct stat fileStats;
	if(stat(fname, &fileStats) == 0){
		execv(fname, argv);
	}

	char* path = getenv("PATH");
	char *base;
	char pathTemp[1024];

	base = strtok(path, ":");

	while(base) {
		strcpy(pathTemp, base);
		if(pathTemp[strlen(pathTemp)-1] != '/'){
			strcat(pathTemp, "/");
		}
		strcat(pathTemp, fname);

		if(stat(pathTemp, &fileStats) == 0){
			execv(pathTemp, argv);
		}
		base = strtok(NULL, ":");
	}

	printf("Cannot find the file\n");
	exit(1);
}

void parse(){
	char* cmd;
	char* args[BUFFER_SIZE];
	int i = 0;

	cmd = strtok(buffer, whiteSpace);
	if(cmd == NULL){
		return; //skip the line if it is empty
	}
	args[i] = cmd;
	i++;
	while(args[i] = strtok(NULL, whiteSpace)){
		i++;
	}
	args[i] = NULL;
	
	childPid = fork();

	if(childPid == -1) {
		perror("Fork error");
		return;
	}

	if(childPid) {
		//parent
		int childStatus;
		
		//Wait until the child is either “interrupted” or “stopped”
		if( waitpid(childPid, &childStatus, WUNTRACED) == -1 ){
			perror("Error");
		}
	} else {
		//child
		signal(SIGINT, SIG_IGN);
		signal(SIGTSTP, SIG_IGN); //reset to default to receive the signals in child 
		searchAndExec(cmd, args);
	}
}


int main() {
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN); //start to ignore signals in parent
	printf("%% ");
	while(fgets(buffer, BUFFER_SIZE, stdin)) {
		int length = strlen(buffer);
		if(length > 0 && buffer[length-1] != '\n') { //check overflow
			printf("Error: Command can only be at most %d characters\n", BUFFER_SIZE);
			printf("%% ");
			continue;
		}
		parse();
		printf("%% ");
	}

	return 0;
}
