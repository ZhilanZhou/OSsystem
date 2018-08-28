//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

/*
 * Read from stdin, change '**' to '^' and write to stdout for the next process outputProcessing.
 */

#include <stdio.h>

int main() {
	char c, nextc;

	while((c = getchar()) != EOF) {
		if(c == '*') {
			if((nextc = getchar()) != EOF) {
				if(nextc == '*') {
					printf("%c", '^');
				} else {
					printf("%c", c);
					printf("%c", nextc);
				}
			}else {
				printf("%c", c);
			}
		} else {
			printf("%c", c);
		}
	}

	return 0;
}