#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){

    printf("1170701 - %s\n", __FILE__);
	char *ptr = malloc(8);

	strcpy(ptr, "pai");

	pid_t r = fork();

	if (r == 0){
		strcpy(ptr, "filho");
		printf("conteúdo do filho: %s\n", ptr);
		exit(0);
	}

	waitpid(r, NULL, 0);
	printf("Conteúdo no pai: %s\n",ptr);
	return 0;
}

