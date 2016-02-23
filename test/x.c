#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
	int i = 0;
	for(;i < 20; i++){
		printf("%d\n",(random() % 2 + 1));
	}
}
