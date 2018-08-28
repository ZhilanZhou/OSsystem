//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

/*
 * Read from stdin, change '\n' to space and write to stdout for the next process.
 * 
 * In this pipeline version, the first step in the original solition is omitted
 * as it does not make much sense to read from stdin then write to stdout directly.
 */

#include <stdio.h>

int main() {
	char c;
	while((c = getchar()) != EOF) {
		if(c == '\n'){
			c = ' ';
		}
		printf("%c", c);
	}
	return 0;
}
