
#include <stdio.h>
#include <libknp.h>

int main(int argc, const char *argv[]) {
	// TODO parse command line args
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");

	init();

	while(1) {
		run();
	}
}

