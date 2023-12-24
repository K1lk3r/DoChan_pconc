#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void rotina_print()
{
	int aux;
	//read do pipe
	//print do pipe
	//sair
	
}



int main()
{
	pthread_t thread_id;
	int pipe_fd[2];
	if(pipe(pipe_fd)==-1){
		printf("grande merda");
	}
	pthread_create(thread_id,NULL,/*rotina*/,NULL);

	return 0;
}
