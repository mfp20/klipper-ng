
#ifndef PROTOCOL_FIRMATA_H
#define PROTOCOL_FIRMATA_H

//#include <stdio.h> // for size_t
//#include <stdbool.h> // for bool
//#include <inttypes.h> // for uint8_t
#include <Hal.hpp>

// Version numbers for the protocol.
#define FIRMATA_PROTOCOL_MAJOR_VERSION  2 // for non-compatible changes
#define FIRMATA_PROTOCOL_MINOR_VERSION  6 // for backwards compatible changes
#define FIRMATA_PROTOCOL_BUGFIX_VERSION 0 // for bugfix releases

// Version numbers for the Firmata library.
#define FIRMATA_FIRMWARE_MAJOR_VERSION  2 // for non-compatible changes
#define FIRMATA_FIRMWARE_MINOR_VERSION  10 // for backwards compatible changes
#define FIRMATA_FIRMWARE_BUGFIX_VERSION 1 // for bugfix releases

// max number of data bytes in incoming messages
#define MAX_DATA_BYTES          64

// message command bytes (128-255/0x80-0xFF)
#define DIGITAL_MESSAGE         0x90 // send data for a digital pin
#define ANALOG_MESSAGE          0xE0 // send data for an analog pin (or PWM)
#define REPORT_ANALOG           0xC0 // enable analog input by pin #
#define REPORT_DIGITAL          0xD0 // enable digital input by port pair
//
#define SET_PIN_MODE            0xF4 // set a pin to INPUT/OUTPUT/PWM/etc
#define SET_DIGITAL_PIN_VALUE   0xF5 // set value of an individual digital pin
//
#define REPORT_VERSION          0xF9 // report protocol version
#define SYSTEM_RESET            0xFF // reset from MIDI
//
#define START_SYSEX             0xF0 // start a MIDI Sysex message
#define END_SYSEX               0xF7 // end a MIDI Sysex message

// extended command set using sysex (0-127/0x00-0x7F)
// 0x00-0x0F reserved for user-defined commands
#define SERIAL_MESSAGE          0x60 // communicate with serial devices, including other boards
#define ENCODER_DATA            0x61 // reply with encoders current positions
#define ACCELSTEPPER_DATA       0x62 // control a stepper motor
#define SERVO_CONFIG            0x70 // set max angle, minPulse, maxPulse, freq
#define STRING_DATA             0x71 // a string message with 14-bits per char
#define STEPPER_DATA            0x72 // control a stepper motor
#define ONEWIRE_DATA            0x73 // send an OneWire read/write/reset/select/skip/search request
#define SHIFT_DATA              0x75 // a bitstream to/from a shift register
#define I2C_REQUEST             0x76 // send an I2C read/write request
#define I2C_REPLY               0x77 // a reply to an I2C read request
#define I2C_CONFIG              0x78 // config I2C settings such as delay times and power pins
#define EXTENDED_ANALOG         0x6F // analog write (PWM, Servo, etc) to any pin
#define PIN_STATE_QUERY         0x6D // ask for a pin's current mode and value
#define PIN_STATE_RESPONSE      0x6E // reply with pin's current mode and value
#define CAPABILITY_QUERY        0x6B // ask for supported modes and resolution of all pins
#define CAPABILITY_RESPONSE     0x6C // reply with supported modes and resolution
#define ANALOG_MAPPING_QUERY    0x69 // ask for mapping of analog to pin numbers
#define ANALOG_MAPPING_RESPONSE 0x6A // reply with mapping info
#define REPORT_FIRMWARE         0x79 // report name and version of the firmware
#define SAMPLING_INTERVAL       0x7A // set the poll rate of the main loop
#define SCHEDULER_DATA          0x7B // send a createtask/deletetask/addtotask/schedule/querytasks/querytask request to the scheduler
#define SYSEX_NON_REALTIME      0x7E // MIDI Reserved for non-realtime messages
#define SYSEX_REALTIME          0x7F // MIDI Reserved for realtime messages

// pin modes
#define INPUT			0x00 // 
#define OUTPUT			0x01 // 
#define PIN_MODE_ANALOG         0x02 // analog pin in analogInput mode
#define PIN_MODE_PWM            0x03 // digital pin in PWM output mode
#define PIN_MODE_SERVO          0x04 // digital pin in Servo output mode
#define PIN_MODE_SHIFT          0x05 // shiftIn/shiftOut mode
#define PIN_MODE_I2C            0x06 // pin included in I2C setup
#define PIN_MODE_ONEWIRE        0x07 // pin configured for 1-wire
#define PIN_MODE_STEPPER        0x08 // pin configured for stepper motor
#define PIN_MODE_ENCODER        0x09 // pin configured for rotary encoders
#define PIN_MODE_SERIAL         0x0A // pin configured for serial communication
#define PIN_MODE_PULLUP         0x0B // enable internal pull-up resistor for pin
#define PIN_MODE_IGNORE         0x7F // pin configured to be ignored by digitalWrite and capabilityResponse
#define TOTAL_PIN_MODES         13

extern "C" {
	// callback function types
	typedef void (*callbackFunction)(uint8_t, int);
	typedef void (*systemResetCallbackFunction)(void);
	typedef void (*stringCallbackFunction)(char *);
	typedef void (*sysexCallbackFunction)(uint8_t command, uint8_t argc, uint8_t *argv);
	typedef void (*delayTaskCallbackFunction)(long delay);
}

class Firmata : public SerialStream {
	public:
		Firmata();
		// SerialStream constructors
		void begin(void);
		void begin(long);
		void begin(SerialStream &s);
		// querying functions
		void printVersion(void);
		void blinkVersion(void);
		void disableBlinkVersion();
		void printFirmwareVersion(void);
		void setFirmwareVersion(uint8_t major, uint8_t minor);
		void setFirmwareNameAndVersion(const char *name, uint8_t major, uint8_t minor);
		// serial receive handling
		int available(void);
		void processInput(void);
		void parse(unsigned char value);
		bool isParsingMessage(void);
		bool isResetting(void);
		// serial send handling
		void sendAnalog(uint8_t pin, int value);
		void sendDigital(uint8_t pin, int value); // TODO implement this
		void sendDigitalPort(uint8_t portNumber, int portData);
		void sendString(const char *string);
		void sendString(uint8_t command, const char *string);
		void sendSysex(uint8_t command, uint8_t bytec, uint8_t *bytev);
		size_t write(uint8_t c);
		// attach & detach callback functions to messages
		void attach(uint8_t command, callbackFunction newFunction);
		void attach(uint8_t command, systemResetCallbackFunction newFunction);
		void attach(uint8_t command, stringCallbackFunction newFunction);
		void attach(uint8_t command, sysexCallbackFunction newFunction);
		void detach(uint8_t command);
		// delegate to Scheduler (if any)
		void attachDelayTask(delayTaskCallbackFunction newFunction);
		void delayTask(long delay);
		// access pin config
		uint8_t getPinMode(uint8_t pin);
		void setPinMode(uint8_t pin, uint8_t config);
		// access pin state
		int getPinState(uint8_t pin);
		void setPinState(uint8_t pin, int state);
		// utility methods
		void sendValueAsTwo7bitBytes(int value);
		void startSysex(void);
		void endSysex(void);
	private:
		// firmware name and version
		uint8_t firmwareVersionCount;
		uint8_t *firmwareVersionVector;
		// input message handling
		uint8_t waitForData; // this flag says the next serial input will be data
		uint8_t executeMultiByteCommand; // execute this after getting multi-byte data
		uint8_t multiByteChannel; // channel data for multiByteCommands
		uint8_t storedInputData[MAX_DATA_BYTES]; // multi-byte data
		// sysex
		bool parsingSysex;
		int sysexBytesRead;
		// pins configuration
		uint8_t pinConfig[TOTAL_PINS]; // configuration of every pin
		int pinState[TOTAL_PINS];	// any value that has been written
		//
		bool resetting;

		// callback functions
		callbackFunction currentAnalogCallback;
		callbackFunction currentDigitalCallback;
		callbackFunction currentReportAnalogCallback;
		callbackFunction currentReportDigitalCallback;
		callbackFunction currentPinModeCallback;
		callbackFunction currentPinValueCallback;
		systemResetCallbackFunction currentSystemResetCallback;
		stringCallbackFunction currentStringCallback;
		sysexCallbackFunction currentSysexCallback;
		delayTaskCallbackFunction delayTaskCallback;

		//
		void processSysexMessage(void);
		void systemReset(void);
		bool blinkVersionDisabled;
		void strobeBlinkPin(uint8_t pin, int count, int onInterval, int offInterval);
};

extern Hal hal;
extern Firmata firmata;

#endif

