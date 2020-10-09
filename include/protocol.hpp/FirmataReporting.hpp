
#ifndef PROTOCOL_REPORTING_H
#define PROTOCOL_REPORTING_H

#include <protocol/Firmata.hpp>
#include <protocol/FirmataFeature.hpp>

#define MINIMUM_SAMPLING_INTERVAL 1

class FirmataReporting: public FirmataFeature {
	public:
		void setSamplingInterval(int interval);
		void handleCapability(uint8_t pin); // empty method
		bool handlePinMode(uint8_t pin, int mode); // empty method
		bool handleSysex(uint8_t command, uint8_t argc, uint8_t* argv);
		bool elapsed();
		void reset();
	private:
		// timer variables
		unsigned long currentMillis;	// store the current value from millis()
		unsigned long previousMillis;	// for comparison with currentMillis
		unsigned int samplingInterval;	// how often to run the main loop (in ms)
};

#endif

