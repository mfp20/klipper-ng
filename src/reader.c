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

int main(int argc, char *argv[]) {
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");
	signal(SIGINT, sigint);

	char fname[] = "/tmp/klipper-ng-pty3";
	uint8_t rx_data[BLOCK_SIZE];

	int ret = initFdThread();
	if (ret < 0) {
		printf("Can't init fdthread. (%d)\n", ret);
		exit(1);
	}

    int fdin = fdOpen(fname, FD_TYPE_FILE);
	int fdstdin = fdOpen("/dev/stdin", FD_TYPE_FILE);
	int fdstdout = fdOpen("/dev/stdout", FD_TYPE_FILE);
	int fdstderr = fdOpen("/dev/stderr", FD_TYPE_FILE);
    if (fdin < 0) {
		printf("fdOpen %s failed. (%d)\n", fname, fdin);
		exit(1);
    }

	while(1) {
		int len = fdRead(fdin, (uint8_t*)rx_data);
		if (len < 0) {
			printf("fdRead %s failed. (%d)\n", fname, len);
			exit(1);
		}
		rx_data[len] = atoi("\0");
		printf("r %d\n", len);
		if (len>0) {
			for(int i=0;i<len;i++) {
				printf("%x ", rx_data[i]);
			}
		}
		printf("\n");
		for(int i=0;i<BLOCK_SIZE;i++) {
			rx_data[i] = 0;
		}
		usleep(500000);
	}
	closeFdThread();
}

