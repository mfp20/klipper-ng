#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <utility/fdthread.linux.h>

volatile sig_atomic_t running = 1;

void sigint(int sig){
	printf("- SIGINT received. Halting...\n");
	running = 0;
}

int main(int argc, const char *argv[]) {
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");
	signal(SIGINT, sigint);

	initFdThread();
	while(running) {
		sleep(1);
	}
	closeFdThread();
	exit(0);
}

