
#ifdef __GIT_REVPARSE__
#define RELEASE_FIRMWARE __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>		// usleep
#include <signal.h>
#include <libknp.h>

volatile sig_atomic_t running = 1;
volatile sig_atomic_t reset = 0;

void sigint(int sig){
	stdoutPrint("- SIGINT received. Halting...\n");
	running = 0;
	reset = 1;
}

int main(int argc, const char *argv[]) {
	// TODO parse command line args
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");

	// intercept SIGINT
	signal(SIGINT, sigint);

	while(running) {
		// init libknp
		init(NULL, NULL);
		printf("- Boot OK. Running...\n");
		// loop run libknp
		while((!reset)&&running) {
			reset = run();
		}
		sleep(1);
		// reset libknp
		if (reset&&running) {
			stdoutPrint("- Reset requested. Resetting...\n");
			eventSystemReset(SYSTEM_RESET_REBOOT);
			reset = 0;
		}
	}
	eventSystemReset(SYSTEM_RESET_HALT);
	sleep(1);
	exit(0);
}

