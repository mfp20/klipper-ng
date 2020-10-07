
#include <Libknp.hpp>

Hal hal;
Firmata firmata;
FirmataExt fExt;
FirmataReporting fRep;
Firmata7BitEnc fEnc;
FirmataScheduler fSched;

void init(void) {
	// init hardware
	hal.init();
	// setup infos
	firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
	// add features
	fExt.addFeature(fRep);
	fExt.addFeature(fSched);
	// register systemResetCallback
	firmata.attach(SYSTEM_RESET, systemResetCallback);
	// reset to default config
	firmata.parse(SYSTEM_RESET);
	//printf("Libknp.cpp, init OK\n");
}

uint16_t run(uint16_t utimeout) {
	// start
	uint16_t ustart = micros();
	
	// hardware tasks
	hal.run();

	// evaluate elapsed time and set remaining time to utimeout
	uint16_t uend = micros();
	uint16_t elapsed = 0;
	if (uend>=ustart) {
		elapsed = uend-ustart;
		//printf("start %d end %d elapsed %d (us)\n", ustart, uend, elapsed);
	} else {
		elapsed = ((999-ustart)+uend);
		//printf("start %d end %d elapsed %d (us) (ovf)\n", ustart, uend, elapsed);
	}
	if (elapsed > utimeout) return elapsed;
	else utimeout = utimeout - elapsed;

	// receive comms as soon as possible, while still checking digital inputs.
	while (firmata.available()) {
		firmata.processInput();
		if (!firmata.isParsingMessage()) {
			break;
		}
	}

	// evaluate elapsed time and set remaining time to utimeout
	uend = micros();
	if (uend>=ustart) {
		elapsed = uend-ustart;
		//printf("start %d end %d elapsed %d (us)\n", ustart, uend, elapsed);
	} else {
		elapsed = ((999-ustart)+uend);
		//printf("start %d end %d elapsed %d (us) (ovf)\n", ustart, uend, elapsed);
	}
	if (elapsed > utimeout) return elapsed;
	else utimeout = utimeout - elapsed;

	// run tasks
	if (!firmata.isParsingMessage()) {
		fSched.runTasks();
	}
	// send comms
	// TODO: make sure that the stream buffer doesn't go over 60 bytes. 
	//		use a timer to sending an event character every 4 ms to trigger the buffer to dump.
	if (fRep.elapsed()) {
	}

	usleep(234); // TODO: dummy delay, to be removed
	// evaluate elapsed time
	uend = micros();
	if (uend>=ustart) {
		elapsed = uend-ustart;
	} else {
		elapsed = ((999-ustart)+uend);
	}
	return elapsed;
}

// initialize a default state
void systemResetCallback(void) {
	// pins with analog capability default to analog input
	// otherwise, pins default to digital output
	for (uint8_t i = 0; i < TOTAL_PINS; i++) {
		if (IS_PIN_ANALOG(i)) {
		} else if (IS_PIN_DIGITAL(i)) {
		}
	}
	// reset capabilities
	fExt.reset();
}

