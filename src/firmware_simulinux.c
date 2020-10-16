
#include <stdio.h>
#include <libknp.h>

int main(int argc, const char *argv[]) {
	// TODO parse command line args
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");

	// init libknp
	initDevice();

	// loop run libknp
	uint16_t last = 0, counter = 0;
	while(1) {
		counter++;
		if ((counter>=last+1000)||((last>64000)&&(counter<2000))) {
			if ((last>64000)&&(counter<2000)) {
				if((((65535-last)*1000)+counter)>1000) last = counter;
			} else {
				last = counter;
				printf("TICK %d\n", last);
			}
		}
		runDevice();
	}
}
