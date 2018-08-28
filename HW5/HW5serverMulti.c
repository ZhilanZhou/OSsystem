//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

/* 
 * A server program being part of emulating a shell. It takes a port as argument.
 * This server can receive multiple connections from different clients, 
 * so it can only be closed when explicitly killing it, like using ^C.
 * When a client is in connection, all inputs are directly from the client via socket.
 * Then it process the inputs like what shell does, including fork a child process.
 */

#include <stdlib.h> 
#include <stdio.h> 
#include <stdarg.h> 
#include <string.h> 
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
void shell_service(void);
void send_output(char * file_name);

int main(int argc, char * argv[]) {
    pid_t spid, term_pid; /* pid_t is typedef for Linux process ID */
    int chld_status;
    bool forever = true;

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

    /* The daemon infinite loop begins here; terminate with external action*/
    while (forever) {
        /* accept an incoming client connection; blocks the
         * process until a connection attempt by a client.
         * creates a new data transfer socket.
         */
        connect_socket = ServerSocket_accept(welcome_socket);
        if (connect_socket < 0) {
            printf("Failed accept on server socket\n");
            exit(-1);
        }
        spid = fork(); /* create child == service process */
        if (spid == -1) {
            perror("fork");
            exit(-1);
        }
        if (spid == 0) { /* code for the service process */
            shell_service();
            Socket_close(connect_socket);
            exit(0);
        } /* end service process */
        else { /* daemon process closes its connect socket */
            Socket_close(connect_socket);
            /* reap a zombie every time through the loop, avoid blocking*/
            term_pid = waitpid(-1, & chld_status, WNOHANG);
        }
    } /* end of infinite loop for daemon process */
}

void shell_service(void) {
    int i, c, rc;
    int childStatus;
    bool forever = true;
    FILE *fp;
    char file_name[MAXFILENAME];
    char line_data[MAX_LINE];
    pid_t childPid, termPid;

    /* will not use the server socket */
    Socket_close(welcome_socket);

    sprintf(file_name, "tmp%i", getpid());
    

    while (forever) { /* actually, until EOF on connect socket */
        /* get a null-terminated string from the client on the data
         * transfer socket up to the maximim input line size.  Continue 
         * getting strings from the client until EOF on the socket.
         */
        fp = freopen(file_name, "w", stdout);
        if(fp == NULL ){
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
            //Calls a helper function that reads the file and passes to client.
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
    return;
}

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