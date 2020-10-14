#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <inttypes.h>

// protocol version number (starting from v3.0 because I started from Firmata v2.6.0)
#define PROTOCOL_VERSION_MAJOR  3 // for non-compatible changes
#define PROTOCOL_VERSION_MINOR  0 // for backwards compatible changes

// max number of data bytes in incoming messages
#define PROTOCOL_MAX_DATA   56

// 0xxxxxxx: extended command set using data bytes (0-127/0x00-0x7F)
// note: unlimited bytes (< MAX_DATA)
#define SYSEX_EXTEND				0x00 // sysex extension, the next 2 bytes define the extended ID
// DATA (0x01-0x09)
#define SYSEX_DIGITAL_PIN_DATA		0x01 // ability to send the value to any pin,
										// as well as supports sending analog values with any number of bits.
#define SYSEX_ANALOG_PIN_DATA		0x02 // ability to specify the analog reference source
										// as well as the analog read and write resolution.
#define SYSEX_SCHEDULER_DATA		0x03 // scheduler request (see related sub commands)
#define SYSEX_ONEWIRE_DATA			0x04 // 1WIRE communication (see related sub commands)
#define SYSEX_UART_DATA				0x05 // UART communication (see related sub commands)
#define SYSEX_I2C_DATA				0x06 // I2C communication (see related sub commands)
#define SYSEX_SPI_DATA				0x07 // SPI communication (see related sub commands)
#define SYSEX_STRING_DATA			0x08 // encoded string (see related sub commands)
#define SYSEX_CRC_DATA				0x09 // CRC-terminated data (see related sub commands)
// REPORT (0x10-0x19)
#define SYSEX_DIGITAL_PIN_REPORT	0x10 // report of ANY digital input pins
#define SYSEX_ANALOG_PIN_REPORT		0x11 // report of ANY analog input pins
#define SYSEX_VERSION_REPORT		0x12 // report name and version of the firmware
#define SYSEX_FEATURES_REPORT		0x13 // report features supported by the firmware
// REQ/REPLY (0x20-0x29)
#define SYSEX_PINCAPS_REQ			0x20 // ask for supported modes and resolution of all pins
#define SYSEX_PINCAPS_REP			0x21 // reply with supported modes and resolution
#define SYSEX_PINMAP_REQ			0x22 // ask for mapping of analog to pin numbers
#define SYSEX_PINMAP_REP			0x23 // reply with mapping info
#define SYSEX_PINSTATE_REQ			0x24 // ask for a pin's current mode and value
#define SYSEX_PINSTATE_REP			0x25 // reply with pin's current mode and value
#define SYSEX_DEVICE_REQ			0x26 // PROPOSED: Generic Device Driver RPC
#define SYSEX_DEVICE_REP			0x27 // PROPOSED: Generic Device Driver RPC
#define SYSEX_RCSWITCH_IN			0x28 // PROPOSED: bridge to RCSwitch Arduino library
#define SYSEX_RCSWITCH_OUT			0x29 // PROPOSED: bridge to RCSwitch Arduino library
// 0x50-0x6F reserved for user-defined commands
// 0x70-0x7D reserved
#define SYSEX_REALTIME_END			0x7E // MIDI Reserved for non-realtime messages
#define SYSEX_REALTIME_START		0x7F // MIDI Reserved for realtime messages

// 1xxxxxxx: core command set using control bytes (128-255/0x80-0xFF)
// note: 1-3 bytes
#define CMD_PIN_MODE				0x80 // set a pin to INPUT/OUTPUT/PWM/etc
#define CMD_DIGITAL_PORT_DATA		0x90 // data for a digital port (ie: 8 pins)
#define CMD_DIGITAL_PIN_DATA		0xA0 // data for a digital pin
#define CMD_ANALOG_PIN_DATA			0xB0 // data for an analog pin (or PWM)
#define CMD_DIGITAL_PORT_REPORT		0xC0 // enable digital input by port pair
#define CMD_DIGITAL_PIN_REPORT		0xD0 // enable digital input by pin #
#define CMD_ANALOG_PIN_REPORT		0xE0 // enable analog input by pin #
#define CMD_SYSEX_START             0xF0 // start a MIDI Sysex message
#define CMD_MICROSTAMP_REPORT		0xF1 // report number of seconds since boot (or overflow)
#define CMD_TIMESTAMP_REPORT		0xF2 // report number of seconds since boot (or overflow)
#define CMD_VERSION_REPORT			0xF3 // report protocol version
#define CMD_CONGESTION_REPORT		0xF4 //
#define CMD_RPC_START				0xF5 //
#define CMD_ENCODING_SWITCH			0xF6 //
#define CMD_SYSEX_END				0xF7 // end a MIDI Sysex message
#define CMD_EMERGENCY_STOP1			0xF8 // stop activity on pin group 1
#define CMD_EMERGENCY_STOP2			0xF9 // stop activity on pin group 3
#define CMD_EMERGENCY_STOP3			0xFA // stop activity on pin group 3
#define CMD_EMERGENCY_STOP4			0xFB // stop activity on pin group 4
#define CMD_SYSTEM_PAUSE			0xFC // system pause
#define CMD_SYSTEM_RESUME			0xFD // system resume (from pause)
#define CMD_SYSTEM_HALT				0xFE // system halt
#define CMD_SYSTEM_RESET			0xFF // MIDI system reset

// Realtime sub-sysex
#define SYSEX_SUB_REALTIME_SYN				1 // start of synchronous operation
#define SYSEX_SUB_REALTIME_ACK				2 // confirm
#define SYSEX_SUB_REALTIME_NACK				3 // problem
#define SYSEX_SUB_REALTIME_FIN				4 // end of synchronous operation
#define SYSEX_SUB_REALTIME_RST				5 // end of synchronous operation
#define SYSEX_SUB_REALTIME_TIME_US			6 // tell me your microseconds (MSB only)
#define SYSEX_SUB_REALTIME_TIME_RESET		7 // reset your microseconds

// Scheduler sub-sysex
#define SYSEX_SUB_SCHED_CREATE				0
#define SYSEX_SUB_SCHED_DELETE				1
#define SYSEX_SUB_SCHED_ADD					2
#define SYSEX_SUB_SCHED_DELAY				3
#define SYSEX_SUB_SCHED_SCHEDULE			4
#define SYSEX_SUB_SCHED_QUERY_ALL			5
#define SYSEX_SUB_SCHED_QUERY_TASK			6
#define SYSEX_SUB_SCHED_REPLY_QUERY_ALL		7
#define SYSEX_SUB_SCHED_REPLY_QUERY_TASK	8
#define SYSEX_SUB_SCHED_REPLY_ERROR			9
#define SYSEX_SUB_SCHED_RESET				10

#define num7BitOutbytes(a)(((a)*7)>>3)

#include <protocol/firmata_user.h>
#include <stdbool.h>

	// callback function types
	typedef void (*cbf_varg_t)(const char *format, ...);
	typedef void (*cbf_7bit_t)(uint8_t *data, uint8_t count, uint8_t *result);

	extern uint8_t multiByteChannel; // channel data for multiByteCommands
	extern uint8_t storedInputData[PROTOCOL_MAX_DATA]; // msg buffer
	extern uint8_t sysexBytesRead; // buffer size

	// callback functions
	extern cbf_varg_t printString;
	extern cbf_varg_t printErr;
	extern cbf_7bit_t cbEncoder;
	extern cbf_7bit_t cbDecoder;

	// each byte become 2*byte to transmit (ie: bandwidth intensive)
	void encode7bit(uint8_t *data, uint8_t count, uint8_t *result);
	void decode7bit(uint8_t *data, uint8_t count, uint8_t *result);
	// shift bits to reduce the amount of bytes to transmit (ie: CPU intensive)
	void encode7bitCompat(uint8_t *data, uint8_t count, uint8_t *result);
	void decode7bitCompat(uint8_t *data, uint8_t count, uint8_t *result);

	// receive 1 byte and return true if the current msg is complete (ie: ready to parse/store/print)
	uint8_t receiveMsg(uint8_t inputData);
	// returns last msg size
	uint8_t measureMsg(void);
	// store last msg
	void storeMsg(uint8_t *copied);
	// print message
	void printMsg(uint8_t count, uint8_t *msg);

#ifdef __cplusplus
}
#endif

#endif

