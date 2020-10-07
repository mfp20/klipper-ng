
#include <protocol/FirmataReporting.hpp>

void FirmataReporting::setSamplingInterval(int interval) {
	samplingInterval = interval;
}

void FirmataReporting::handleCapability(uint8_t pin) { pin=pin; }

bool FirmataReporting::handlePinMode(uint8_t pin, int mode) {
	pin=pin; // TODO
	mode=mode;
	return false;
}

bool FirmataReporting::handleSysex(uint8_t command, uint8_t argc, uint8_t* argv) {
	if (command == SAMPLING_INTERVAL) {
		if (argc > 1) {
			samplingInterval = argv[0] + (argv[1] << 7);
			if (samplingInterval < MINIMUM_SAMPLING_INTERVAL) {
				samplingInterval = MINIMUM_SAMPLING_INTERVAL;
			}
			return true;
		}
	}
	return false;
}

bool FirmataReporting::elapsed() {
	currentMillis = millis();
	if (currentMillis - previousMillis > samplingInterval) {
		previousMillis += samplingInterval;
		if (currentMillis - previousMillis > samplingInterval)
			previousMillis = currentMillis - samplingInterval;
		return true;
	}
	return false;
}

void FirmataReporting::reset() {
	previousMillis = millis();
	samplingInterval = 19;
}

