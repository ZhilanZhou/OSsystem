//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work


#include <stdlib.h> 
#include <stdio.h> 
#include <stdarg.h> 
#include <string.h> 
#include <errno.h>
#include <ctype.h> 
#include <stdbool.h> 
#include <sys/types.h> 
#include <sys/wait.h>

#include "Socket.h"
#include "HW5common.h" /* definitions shared by client and server */

#define MAXFILENAME 100

/* variables to hold socket descriptors */
ServerSocket welcome_socket;
Socket connect_socket;
const char white_space[]=" \f\n\r\t\v";
void send_output(char * file_name);

int main(int argc, char * argv[]) {
    pid_t childPid, termPid; /* pid_t is typedef for Linux process ID */
    bool forever = true;
    int i, c, rc;
    int childStatus;
    FILE *fp;
    char file_name[MAXFILENAME];
    char line_data[MAX_LINE];

    if (argc < 2) {
        printf("No port specified\n");
        return (-1);
    }

    /* create a "welcoming" socket at the specified port */
    welcome_socket = ServerSocket_new(atoi(argv[1]));
    if (welcome_socket < 0) {
        printf("Failed new server socket\n");
        return (-1);
    }

    connect_socket = ServerSocket_accept(welcome_socket);
    if (connect_socket < 0) {
        printf("Failed accept on server socket\n");
        exit(-1);
    }

    sprintf(file_name, "tmp%i", getpid());
    
    while (forever) { /* actually, until EOF on connect socket */
        /* get a null-terminated string from the client on the data
         * transfer socket up to the maximim input line size.  Continue 
         * getting strings from the client until EOF on the socket.
         */
        fp = freopen(file_name, "w", stdout);
        if(fp == NULL){
            printf("Error in creating temp file\n");
            exit(-1);
        }
        dup2(1,2); //Redirects stderr to stdout, so certain error message can be shown

        for (i = 0; i < MAX_LINE; i++) {
            c = Socket_getc(connect_socket);
            if (c == EOF) {
                printf("Socket_getc EOF or error\n");
                break; /* assume socket EOF ends service for this client */
            } else {
                line_data[i] = c;
                if (c == '\n') {
                    break;
                }
            }
        }
        line_data[i+1] = '\0';

        childPid = fork();

        if (childPid < 0) {
            perror("Fork error");
            exit(-1);
        }

        if (childPid) {
            //parent
            termPid = waitpid(-1, &childStatus, 0);
            if(termPid == -1){
                perror("wait");
            }
            else{
                //checks if the process was terminated normally and passes a response line to the file indicating
                //whether it was or not             
                if(WIFEXITED(childStatus)){
                    printf("%s PID %d exited, with status %d\n", GOODCODE, childPid, WEXITSTATUS(childStatus));
                }
                else{
                    printf("%s PID %d error\n", BADCODE, childPid);
                }
            }
            //Closes redirect           
            fclose(fp);
            send_output(file_name);
        } else {
            //child
            Socket_close(connect_socket);

            char* cmd;
            cmd = strtok(line_data, white_space);

            char* args[MAX_LINE];
            
            args[0] = cmd;
            int i = 1;
            while (args[i] = strtok(NULL, white_space)){
                i++;
            }
            args[i] = NULL;

            if(execvp(args[0], args) < 0){
                perror(args[0]); 
                exit(-1); 
            }
            fclose(fp);
        }
    } /* end while loop of the service process */
    return 0;
}

//A helper function that reads the temp file and writes all characters into the socket until an EOF.
void send_output(char * file_name) {
    FILE * f;
    int c;

    if ((f = fopen(file_name, "r")) == NULL) {
        printf("Server Error opening output file\n");
        exit(-1);
    }

    c = getc(f);

    while (c != EOF) {
        Socket_putc(c, connect_socket);
        c = getc(f);
    }

    Socket_putc('\0', connect_socket);

    fclose(f);
    return;
}