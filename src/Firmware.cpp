
#include <stdio.h>
#include <Libknp.hpp>

int main(int argc, const char *argv[]) {
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");
	//
	init();
	//
	uint16_t counter = 0;
	uint16_t last = 0;
	uint16_t elapsed = 0;
	//logWrite("Firmware.cpp, pre OK\n");
	while(1) {
		elapsed = run(999);
		if (elapsed > 999)
			errWrite("LAG (+%d us)\n", elapsed-999);
		else
			usleep(999-elapsed);
		if ((counter / 1000) > last) {
			last = counter / 1000;
			logWrite("TICK %9d | %5d:%4d | ---\n", counter, seconds(), millis());
			consoleWrite("TICK %9d | %5d:%4d | ---\n", counter, seconds(), millis());
		}
		counter++;
	}
	//logWrite("Firmware.cpp, loop OUT\n");
}

