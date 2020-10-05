
#include <Libknp.hpp>

Firmata firmata;
FirmataExt fExt;
FirmataReporting fRep;
Firmata7BitEnc fEnc;
FirmataScheduler fSched;

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

void setup(void) {
	firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);

	fExt.addFeature(fRep);
	fExt.addFeature(fSched);

	// systemResetCallback is declared here
	firmata.attach(SYSTEM_RESET, systemResetCallback);
	firmata.parse(SYSTEM_RESET);  // reset to default config
}

void loop(void) {
	// STREAMREAD - processing incoming message as soon as possible, while still checking digital inputs.
	while (firmata.available()) {
		firmata.processInput();
		if (!firmata.isParsingMessage()) {
			goto runtasks;
		}
	}
	if (!firmata.isParsingMessage()) {
		runtasks: fSched.runTasks();
	}

	// SEND STREAM WRITE BUFFER
	// TODO: make sure that the stream buffer doesn't go over 60 bytes. use a timer to sending an event character every 4 ms to trigger the buffer to dump.
	if (fRep.elapsed()) {
	}
}

