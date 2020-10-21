
#ifdef __GIT_REVPARSE__
#define RELEASE_FIRMWARE __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#include <libknp.h>
#include <stdio.h>
#include <unistd.h>		// usleep

int main(int argc, const char *argv[]) {
	// TODO parse command line args
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");

	// init libknp
	init(NULL, NULL);
	stdoutPrint("- Boot OK. Running...\n");

	// loop run libknp
	while(1) {
		run();
	}
}

