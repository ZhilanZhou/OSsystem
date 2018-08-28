//@author: Zhilan Zhou
//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work
#include<stdio.h>
main(){
	char currentInput, charTemp='\0';
	char lineOutput[81];
	lineOutput[80]='\0';
	int num = 0;
	int ifEnd = 0;
	currentInput=getchar();
	while (currentInput!=EOF){
		while (num < 80){
			if (currentInput==EOF){
				ifEnd=1;
				break;
			}
			if (currentInput=='\n'){
				lineOutput[num]=' ';
				num++;
				currentInput=getchar();
				continue;
			}
			if (currentInput=='*'){
				currentInput=getchar();
				if (currentInput=='*'){
					lineOutput[num]='^';
					num++;
					currentInput=getchar();
					continue;
				}else{
					lineOutput[num]='*';
					num++;
					if (num == 80){
						charTemp=currentInput;
						currentInput=getchar();
						continue;
					}
				}
			}
			lineOutput[num]=currentInput;
			num++;
			currentInput=getchar();
		}
		if (ifEnd==1){
			break;
		}
		printf("%s\n", lineOutput);
		num = 0;
		if (charTemp!='\0'){
			lineOutput[num]=charTemp;
			charTemp='\0';
			num++;
		}
	}
	return 0;
}
