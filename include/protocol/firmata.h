#ifndef PROTOCOL_FIRMATA_H
#define PROTOCOL_FIRMATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <inttypes.h>

	// firmata protocol version number
#define PROTOCOL_FIRMATA_VERSION_MAJOR  2 // for non-compatible changes
#define PROTOCOL_FIRMATA_VERSION_MINOR  6 // for backwards compatible changes
#define PROTOCOL_FIRMATA_VERSION_BUGFIX 0 // for bugfix releases

	// max number of data bytes in incoming messages
#define PROTOCOL_FIRMATA_MAX_DATA_PER_MSG   57

	// message command bytes (128-255/0x80-0xFF)
#define CMD_DIGITAL_MESSAGE         0x90 // send data for a digital pin
#define CMD_REPORT_ANALOG           0xC0 // enable analog input by pin #
#define CMD_REPORT_DIGITAL          0xD0 // enable digital input by port pair
#define CMD_ANALOG_MESSAGE          0xE0 // send data for an analog pin (or PWM)
#define CMD_START_SYSEX             0xF0 // start a MIDI Sysex message
#define CMD_SET_PIN_MODE            0xF4 // set a pin to INPUT/OUTPUT/PWM/etc
#define CMD_SET_DIGITAL_PIN_VALUE   0xF5 // set value of an individual digital pin
#define CMD_END_SYSEX               0xF7 // end a MIDI Sysex message
#define CMD_REPORT_VERSION          0xF9 // report protocol version
#define CMD_SYSTEM_RESET            0xFF // reset from MIDI

	// extended command set using sysex (0-127/0x00-0x7F)
#define CMD_EID_EXTEND					0x00 // the next 2 bytes define the extended ID
	// 0x01-0x0F reserved for user-defined commands
#define CMD_EID_RCOUT					0x5C // PROPOSED: bridge to RCSwitch Arduino library
#define CMD_EID_RCIN					0x5D // PROPOSED: bridge to RCSwitch Arduino library
#define CMD_EID_DEVICE_QUERY			0x5E // PROPOSED: Generic Device Driver RPC
#define CMD_EID_DEVICE_RESPONSE			0x5F // PROPOSED: Generic Device Driver RPC
#define CMD_EID_SERIAL_MESSAGE			0x60 // communicate with serial devices, including other boards
#define CMD_EID_ENCODER_DATA			0x61 // reply with encoders current positions
#define CMD_EID_ACCELSTEPPER_DATA		0x62 // control a stepper motor
#define CMD_EID_REPORT_DIGITAL_PIN		0x63 // PROPOSED:
#define CMD_EID_EXTENDED_REPORT_ANALOG	0x64 // PROPOSED:
#define CMD_EID_REPORT_FEATURES			0x65 // PROPOSED:
#define CMD_EID_SERIAL_DATA2			0x67 // PROPOSED:
#define CMD_EID_SPI_DATA				0x68 // PROPOSED:
#define CMD_EID_ANALOG_MAPPING_QUERY	0x69 // ask for mapping of analog to pin numbers
#define CMD_EID_ANALOG_MAPPING_RESPONSE	0x6A // reply with mapping info
#define CMD_EID_CAPABILITY_QUERY		0x6B // ask for supported modes and resolution of all pins
#define CMD_EID_CAPABILITY_RESPONSE		0x6C // reply with supported modes and resolution
#define CMD_EID_PIN_STATE_QUERY			0x6D // ask for a pin's current mode and value
#define CMD_EID_PIN_STATE_RESPONSE		0x6E // reply with pin's current mode and value
#define CMD_EID_EXTENDED_ANALOG			0x6F // analog write (PWM, Servo, etc) to any pin
#define CMD_EID_SERVO_CONFIG			0x70 // set max angle, minPulse, maxPulse, freq
#define CMD_EID_STRING_DATA				0x71 // a string message with 14-bits per char
#define CMD_EID_STEPPER_DATA			0x72 // DEPRECATED: control a stepper motor
#define CMD_EID_ONEWIRE_DATA			0x73 // send an OneWire read/write/reset/select/skip/search request
#define CMD_EID_SHIFT_DATA				0x75 // a bitstream to/from a shift register
#define CMD_EID_I2C_REQUEST				0x76 // send an I2C read/write request
#define CMD_EID_I2C_REPLY				0x77 // a reply to an I2C read request
#define CMD_EID_I2C_CONFIG				0x78 // config I2C settings such as delay times and power pins
#define CMD_EID_REPORT_FIRMWARE			0x79 // report name and version of the firmware
#define CMD_EID_SAMPLING_INTERVAL		0x7A // set the poll rate of the main loop
#define CMD_EID_SCHEDULER_DATA			0x7B // send a createtask/deletetask/addtotask/schedule/querytasks/querytask request to the scheduler
#define CMD_EID_ANALOG_CONFIG			0x7C // PROPOSAL: 
#define CMD_EID_NON_REALTIME			0x7E // MIDI Reserved for non-realtime messages
#define CMD_EID_REALTIME				0x7F // MIDI Reserved for realtime messages

	// Scheduler sub-sysex
#define CMD_SUB_SCHED_CREATE		0
#define CMD_SUB_SCHED_DELETE		1
#define CMD_SUB_SCHED_ADD_TO		2
#define CMD_SUB_SCHED_DELAY			3
#define CMD_SUB_SCHED_SCHEDULE		4
#define CMD_SUB_SCHED_QUERY_ALL		5
#define CMD_SUB_SCHED_QUERY_TASK	6
#define CMD_SUB_SCHED_RESET			7
#define CMD_SUB_SCHED_REPLY_ERROR   8
#define CMD_SUB_SCHED_REPLY_QUERY_ALL	9
#define CMD_SUB_SCHED_REPLY_QUERY_TASK	10

#define num7BitOutbytes(a)(((a)*7)>>3)

#include <protocol/firmata_user.h>

	// callback function types
	typedef void (*cbf_string_t)(char *);
	typedef void (*cbf_simple_t)(uint8_t cmd, uint8_t byte1, uint8_t byte2);
	typedef void (*cbf_sysex_t)(uint8_t cmd, uint8_t argc, uint8_t *argv);
	typedef void (*cbf_7bit_t)(uint8_t *data, uint8_t count, uint8_t *result);

	// callback functions
	extern cbf_string_t cbString;
	extern cbf_simple_t cbSimple;
	extern cbf_sysex_t cbSysex;
	extern cbf_7bit_t cbEncoder;
	extern cbf_7bit_t cbDecoder;

	// receive 1 byte and dispatch completed commands when fully received
	void collect(uint8_t inputData);
	// each byte become 2*byte to transmit (ie: bandwidth intensive)
	void encode7bit(uint8_t *data, uint8_t count, uint8_t *result);
	void decode7bit(uint8_t *data, uint8_t count, uint8_t *result);
	// shift bits to reduce the amount of bytes to transmit (ie: CPU intensive)
	void encode7bitCompat(uint8_t *data, uint8_t count, uint8_t *result);
	void decode7bitCompat(uint8_t *data, uint8_t count, uint8_t *result);

#ifdef __cplusplus
}
#endif

#endif

