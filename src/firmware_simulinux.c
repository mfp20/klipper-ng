
#include <stdio.h>
#include <libknp.h>

#ifdef __GIT_REVPARSE__
#define RELEASE_FIRMWARE __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

int main(int argc, const char *argv[]) {
	// TODO parse command line args
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");

	// init libknp
	init(NULL, NULL);

	// loop run libknp
	uint16_t last = 0, counter = 0;
	while(1) {
		counter++;
		if ((counter>=last+1000)||((last>64000)&&(counter<2000))) {
			if ((last>64000)&&(counter<2000)) {
				if((((65535-last)*1000)+counter)>1000) last = counter;
			} else {
				last = counter;
				printf("%d:%d TICK %d, protoErr %d\n", seconds(), millis(), last, protocolErrorNo);
			}
		}
		cmdPinMode(1, 2);
		cmdGetDigitalPort(2, 234);
		cmdSetDigitalPort(3, 234);
		cmdGetDigitalPin(4, 234);
		cmdSetDigitalPin(5, 234);
		cmdGetAnalogPin(6, 234);
		cmdSetAnalogPin(7, 234);
		cmdHandshakeProtocolVersion();
		cmdHandshakeEncoding(0);
		cmdGetInfo(3);
		cmdSendSignal(1, 123);
		cmdSendInterrupt(1, 234);
		cmdEmergencyStop(10);
		cmdSystemPause(123);
		cmdSystemResume(123);
		cmdSystemHalt();
		cmdSystemReset();
		run();
		break;
	}
}

