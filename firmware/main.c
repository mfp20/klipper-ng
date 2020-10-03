#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "sched.h"

int main (int argc, const char * argv[]) {
	printf("main()\n");
	init();
	run();
}

