#include "utility/fdthread.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv) {
	uint8_t rx_data[BLOCK_SIZE];
	char tx_data[] = "Uno due tre prova";

	int ret = initFdThread();
	if (ret < 0) {
		printf("Can't init fdthread. (%d)\n", ret);
		exit(1);
	}
    int fdin = fdOpen("input.txt", FD_TYPE_FILE);
    if (fdin < 0) {
		printf("fdOpen %s failed. (%d)\n", "input.txt", fdin);
		exit(1);
    }
	int fdout = fdOpen("output.txt", FD_TYPE_FILE);
    if (fdout < 0) {
		printf("fdOpen %s failed. (%d)\n", "output.txt", fdout);
		exit(1);
	}

	fdWrite(fdout, (uint8_t*)tx_data, strlen(tx_data));
	printf("w (%d) %s\n", (int)strlen(tx_data), tx_data);
	while(1) {
		int len = fdRead(fdin, (uint8_t*)rx_data);
		if (len < 0) {
			printf("fdRead %s failed. (%d)\n", "output.txt", len);
			exit(1);
		}
		rx_data[len] = atoi("\0");
		if (len>0) printf("r (%d) %s\n", len, rx_data);
		sleep(1);
	}
	closeFdThread();
}

