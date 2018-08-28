//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

/* 
 * A client program being part of emulating a shell. It takes a port and a server as arguments.
 * All inputs are directly sent to the server via socket for processing, 
 * then it takes the response from server, and writes the response to stdout.
 * After that, it can receive new response from stdin.
 * This loop will continue until an EOF is reached.
 */

#include <stdlib.h> 
#include <stdio.h> 
#include <stdarg.h> 
#include <string.h>
#include <stdbool.h> 

#include "Socket.h"
#include "HW5common.h" /* definitions common to client and server */

int main(int argc, char * argv[]) {
    int i, c, rc;
    char line_data[MAX_LINE];

    /* variable to hold socket descriptor */
    Socket connect_socket;

    if (argc < 3) {
        printf("No host and port\n");
        return (-1);
    }

    /* connect to the server at the specified host and port;
     * blocks the process until the connection is accepted
     * by the server; creates a new data transfer socket.
     */
    connect_socket = Socket_new(argv[1], atoi(argv[2]));
    if (connect_socket < 0) {
        printf("Failed to connect to server\n");
        return (-1);
    }

    printf("%% ");

    /* Get a string from stdin up to and including 
     * a newline or to the maximim input line size.  
     * Continue getting strings from stdin until EOF.
     */
    while ((fgets(line_data, sizeof(line_data), stdin) != NULL)) {
        int length = strlen(line_data);

        if (length > 0 && line_data[length - 1] != '\n') { //Check overflow
            printf("Command is too long\n");
            printf("%% ");
            continue;
        }

        /* Send the characters of the input line to the server
         * using the data transfer socket.
         */
        for (i = 0; i < length; i++) {
            c = line_data[i];
            rc = Socket_putc(c, connect_socket);
            if (rc == EOF) {
                printf("input EOF or error\n");
                Socket_close(connect_socket);
                exit(-1); /* Assume socket problem is fatal, end client */
            }
        }

        bool continue_receving = true;
        char temp_output[MAX_LINE + 1];

        /* 
         * This loops continuously gets char from server until a response line is met.
         * And output response line only when it is bad.
         */
        while (continue_receving) {
            int j = 0;
            c = Socket_getc(connect_socket);

            /* Receive the characters of response from the server
             * using the data transfer socket.
             */
            while (c != '\n' && c != '\0') {
                if (c == EOF) {
                    printf("output EOF or error\n");
                }
                temp_output[j] = c;
                j++;
                c = Socket_getc(connect_socket);
            }

            temp_output[j] = '\n';
            temp_output[j + 1] = '\0'; //Make sure the string has an end.

            /* 
             * Check the response line
             */
            if (strstr(temp_output, BADCODE) != NULL) { //Bad response
                printf("%s", temp_output);
                continue_receving = false;
            } else if (strstr(temp_output, GOODCODE) != NULL) { //Good response
                continue_receving = false;
            } else {
                if (temp_output[0] != '\n') {
                    printf("%s", temp_output);
                }
            }
        }
        printf("%% ");
    } /* End of while loop; at EOF */
    Socket_close(connect_socket);
    exit(0);
}