
#ifndef PROTOCOL_FEATURE_H
#define PROTOCOL_FEATURE_H

#include <protocol/Firmata.hpp>

class FirmataFeature {
	public:
		virtual void handleCapability(uint8_t pin) = 0;
		virtual bool handlePinMode(uint8_t pin, int mode) = 0;
		virtual bool handleSysex(uint8_t command, uint8_t argc, uint8_t* argv) = 0;
		virtual void reset() = 0;
};

#endif

