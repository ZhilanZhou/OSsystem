//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

//About executing this program: as ^C and ^Z are ignored in the main process, please use ^D to stop 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

//size of a single command
#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];
pid_t childPid;
//characters of whitespace
const char whiteSpace[]=" \f\n\r\t\v";

void commandExec(char* cmd, char* args[]) {
	struct stat fileStats;
	if (stat(cmd, &fileStats) == 0){
		execv(cmd, args);
	}

	char* path = getenv("PATH");
	char pathTemp[2048]; //assume the environment path is not longer than 2048 characters
	char *subPath;

	subPath = strtok(path, ":");

	//traverse each possibility for file path
	while (subPath) {
		strcpy(pathTemp, subPath);

		if (pathTemp[strlen(pathTemp)-1] != '/'){
			strcat(pathTemp, "/");
		}
		strcat(pathTemp, cmd); //append filename to current path

		if (stat(pathTemp, &fileStats) == 0){
			execv(pathTemp, args);
		}
		subPath = strtok(NULL, ":");
	}
	//loop finishes without executing, search fails
	printf("Error: Cannot find the file\n");
	exit(1);
}

int main() {
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN); //start to ignore ^C and ^Z signals in parent

	printf("%% ");

	while (fgets(buffer, BUFFER_SIZE, stdin)) {
		int length = strlen(buffer);

		if (length > 0 && buffer[length-1] != '\n') { //check overflow
			printf("Error: Command is too long\n");
			printf("%% ");
			continue;
		}
		char* cmd;
	
		cmd = strtok(buffer, whiteSpace);
		if (cmd == NULL){
			printf("%% ");
			continue; //skip the line if it is empty
		}
	
		childPid = fork();

		if (childPid == -1) {
			perror("Fork error");
			printf("%% ");
			continue;
		}

		if (childPid) {
			//parent
			int childStatus;
		
			//Wait until the child is either “interrupted” or “stopped”
			if (waitpid(childPid, &childStatus, WUNTRACED) == -1){
				perror("Error");
			}
		} else {
			//child
			signal(SIGINT, SIG_IGN);
			signal(SIGTSTP, SIG_IGN); //reset to default to receive ^C and ^Z the signals in child 

			char* args[BUFFER_SIZE];
			
			args[0] = cmd;
			int i = 1;
			while (args[i] = strtok(NULL, whiteSpace)){
				i++;
			}
			args[i] = NULL;

			commandExec(cmd, args);
		}
		printf("%% ");
	}
	return 0;
}
