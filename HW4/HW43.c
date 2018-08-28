//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work

/*
 * Read from stdin, print out 80 characters lines.
 */

#include <stdio.h>

#define LINE_LENGTH 80

int main() {
	int count = 0;
	char line[LINE_LENGTH + 1];
	char c;
	line[LINE_LENGTH] = '\0';

	while((c = getchar()) != EOF) {
		line[count] = c;
		count++;
		if(count == LINE_LENGTH) {
			printf("%s\n", line);
			count = 0;
		}
	}
	return 0;
}