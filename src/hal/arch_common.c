
#include <hal/arch.h>

uint16_t uelapsed(uint16_t mstart, uint16_t ustart, uint16_t uend, uint16_t mend) {
	// less than 1ms, within the same millisecond interval
	if (mstart==mend) {
		if (uend>ustart) {
			return uend-ustart;
		} else {
			return (1000-ustart)+uend;
		}
		// less than 1ms, across 2ms interval
	} else if (mend==mstart+1) {
		if (uend>ustart) {
			return (1000-ustart)+uend;
		} else {
			return (1000-ustart)+uend;
		}
		// more than 1ms
	} else {
		if (mend>mstart) {
			return ((mend-mstart-1)*1000)+((1000-ustart)+uend);
		} else {
			return (((1000-mstart)+mend+1)*1000)+((1000-ustart)+uend);
		}
	}
}

void vars_reset(void) {
	//
	for (uint8_t i = 0;i<8;i++) {
		adc_enable[i] = 0;
		adc_value[i] = 0;
	}
	//
	for (uint8_t i = 0;i<8;i++) {
		pulsing.pin[i] = 0;
	}
	pulsing.head = NULL;
	pulsing.tail = NULL;
	//printf("vars_reset() OK\n");
}

void commports_reset(void) {
	if (ports != NULL)
		for (int i=0;i<=ports_no;i++) {
			if (ports[i].fd != 0) {
				//printf("port %d, fd %d", i, ports[i].fd);
				ports[i].end(&ports[i]);
			}
		}
	//if (ports != NULL) free(ports);
	ports = (commport_t *)realloc(ports, sizeof(commport_t));
	ports_no = 0;
	console = NULL;
	//printf("commports_reset() OK\n");
}

void arch_init(void) {
	vars_reset();
	commports_reset();
	// arch specific
	_arch_init();
	//printf("arch_init OK\n");
}

void arch_run(void) {
	_arch_run();
}

uint8_t arch_reset(void) {
	_arch_reset();
	commports_reset();
	vars_reset();
	return 0;
}

uint8_t arch_halt(void) {
	arch_reset();
	return 0;
}

