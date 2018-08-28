//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

/* 
 * A client program that uses the service of converting a string
 * characters to upper case.
 *
 * This program reads input strings from stdin and sends them to
 * a server which converts the characters to upper case and 
 * returns the converted string back to the client.  The client
 * then puts the converted string to stdout.
 *
 * The client has two positional parameters: (1) the DNS host name 
 * where the server program is running, and (2) the port number of 
 * the server's "welcoming" socket.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Socket.h"
#include "ToUpper.h" /* definitions common to client and server */

int main(int argc, char* argv[]) {
  int i, c, rc;
  int count = 0;

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

  /* get a string from stdin up to and including 
   * a newline or to the maximim input line size.  
   * Continue getting strings from stdin until EOF.
   */ 
 printf("%% ");

 while ((fgets(line_data, sizeof(line_data), stdin) != NULL)) {
  int length = strlen(line_data);

  if (length > 0 && line_data[length-1] != '\n') { //check overflow
    printf("Error: Command is too long\n");
    printf("%% ");
    continue;
  }

      /* send the characters of the input line to the server
       * using the data transfer socket.
       */
  for (i = 0; i < count; i++) {
   c = line_data[i];
   rc = Socket_putc(c, connect_socket);
   if (rc == EOF) {
     printf("Socket_putc EOF or error\n");             
     Socket_close(connect_socket);
              exit (-1);  /* assume socket problem is fatal, end client */
   }
 }
 bool continue_receving = true;
 char temp_output[MAX_LINE+1];
 while (continue_receving) {
  int j = 0;
  c = Socket_getc(connect_socket);
  while(c != '\n' && c != '\0') {
    if (c == EOF) {
      printf("Socket_getc EOF or error\n");             
    }
    temp_output[j] = c;
    j++;
    c = Socket_getc(connect_socket);
  }
  temp_output[j] = '\n';
  temp_output[j+1] = '\0';
  if(strstr(temp_output, BADSIGNAL) != NULL) {
    printf("%s", temp_output);
    continue_receving = false;
  }else if(strstr(temp_output, GOODSIGNAL) != NULL) {
    continue_receving = false;
  }else {
    printf("%s", temp_output);
  }
 }
printf("%% ");
 } /* end of while loop; at EOF */
Socket_close(connect_socket);
exit(0);
}
