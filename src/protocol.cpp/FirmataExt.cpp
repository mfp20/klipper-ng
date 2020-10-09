
#include <protocol/FirmataExt.hpp>

void handleSetPinModeCallback(uint8_t pin, int mode) {
	if (!fExt.handlePinMode(pin, mode) && mode != PIN_MODE_IGNORE) {
		firmata.sendString("Unknown pin mode"); // TODO: put error msgs in EEPROM
	}
}
void handleSysexCallback(uint8_t command, uint8_t argc, uint8_t* argv) {
	if (!fExt.handleSysex(command, argc, argv)) {
		firmata.sendString("Unhandled sysex command");
	}
}

FirmataExt::FirmataExt() {
	firmata.attach(SET_PIN_MODE, handleSetPinModeCallback);
	firmata.attach((uint8_t)START_SYSEX, handleSysexCallback);
	numFeatures = 0;
}

void FirmataExt::handleCapability(uint8_t pin) { pin=pin; }

bool FirmataExt::handlePinMode(uint8_t pin, int mode) {
	bool known = false;
	for (uint8_t i = 0; i < numFeatures; i++) {
		known |= features[i]->handlePinMode(pin, mode);
	}
	return known;
}

bool FirmataExt::handleSysex(uint8_t command, uint8_t argc, uint8_t* argv) {
	switch (command) {
		case PIN_STATE_QUERY:
			if (argc > 0) {
				uint8_t pin = argv[0];
				if (pin < TOTAL_PINS) {
					firmata.write(START_SYSEX);
					firmata.write(PIN_STATE_RESPONSE);
					firmata.write(pin);
					firmata.write(firmata.getPinMode(pin));
					int pinState = firmata.getPinState(pin);
					firmata.write((uint8_t)pinState & 0x7F);
					if (pinState & 0xFF80) firmata.write((uint8_t)(pinState >> 7) & 0x7F);
					if (pinState & 0xC000) firmata.write((uint8_t)(pinState >> 14) & 0x7F);
					firmata.write(END_SYSEX);
					return true;
				}
			}
			break;
		case CAPABILITY_QUERY:
			firmata.write(START_SYSEX);
			firmata.write(CAPABILITY_RESPONSE);
			for (uint8_t pin = 0; pin < TOTAL_PINS; pin++) {
				if (firmata.getPinMode(pin) != PIN_MODE_IGNORE) {
					for (uint8_t i = 0; i < numFeatures; i++) {
						features[i]->handleCapability(pin);
					}
				}
				firmata.write(127);
			}
			firmata.write(END_SYSEX);
			return true;
		default:
			for (uint8_t i = 0; i < numFeatures; i++) {
				if (features[i]->handleSysex(command, argc, argv)) {
					return true;
				}
			}
			break;
	}
	return false;
}

void FirmataExt::addFeature(FirmataFeature &capability) {
	if (numFeatures < MAX_FEATURES) {
		features[numFeatures++] = &capability;
	}
}

void FirmataExt::reset() {
	for (uint8_t i = 0; i < numFeatures; i++) {
		features[i]->reset();
	}
}

