#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <libknp.h>

volatile sig_atomic_t running = 1;

void sigint(int sig){
	stdoutPrint("- SIGINT received. Halting...\n");
	running = 0;
}

int main(int argc, const char *argv[]) {
	// TODO parse command line args
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");

	signal(SIGINT, sigint);

	init(NULL, NULL);

	while(running) {
		char output[1000];
		getEvent();
		if (eventSize>0) {
			printf("event size %d\n", eventSize);
			printEvent(0,NULL,output);
			for(int i=0;i<1000;i++) {
				output[i] = 0;
			}
		}
	}
	sleep(1);
	exit(0);
}

