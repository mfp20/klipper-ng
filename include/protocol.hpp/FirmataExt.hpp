
#ifndef PROTOCOL_EXT_H
#define PROTOCOL_EXT_H

#include <protocol/FirmataFeature.hpp>

#define MAX_FEATURES TOTAL_PIN_MODES + 1

void handleSetPinModeCallback(uint8_t pin, int mode);
void handleSysexCallback(uint8_t command, uint8_t argc, uint8_t* argv);

class FirmataExt: public FirmataFeature {
	public:
		FirmataExt();
		void handleCapability(uint8_t pin); //empty method
		bool handlePinMode(uint8_t pin, int mode);
		bool handleSysex(uint8_t command, uint8_t argc, uint8_t* argv);
		void addFeature(FirmataFeature &capability);
		void reset();
	private:
		FirmataFeature *features[MAX_FEATURES];
		uint8_t numFeatures;
};

extern FirmataExt fExt;
extern Firmata firmata;

#endif

