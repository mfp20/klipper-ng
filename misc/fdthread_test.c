#include "utility/fdthread.linux.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv) {
	char fname[] = "/tmp/klipper-ng-pty3";
	uint8_t rx_data[BLOCK_SIZE];

	int ret = initFdThread();
	if (ret < 0) {
		printf("Can't init fdthread. (%d)\n", ret);
		exit(1);
	}

    int fdin = fdOpen(fname, FD_TYPE_FILE);
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

