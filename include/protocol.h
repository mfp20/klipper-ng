#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>
#include <protocol_custom.h>

/*
The protocol mimic the MIDI protocol. Each byte of the MIDI byte stream
starts with MSb 1 for control bytes and starts with MSb 0 for data bytes.
A MIDI byte stream is framed in 'events':

	event = [ticks][message]

		ticks = [0xFF][2 bytes delta time] (the time delay from the previous event)

		message = [status_byte][status_data] (new event definition)

Common events are 3 bytes long, ie: have max 2 bytes of data.
To extend the amount of data in each event, the status byte can be 0xF0 to start a sysex.
Sysex have virtually unlimited bytes of data.
After 'sysex start' (0xF0):

			sysex = [mod_byte][sysex_byte][data][0xF7]
				sysex = [0x70][extended_sysex_byte][data][0xF7]
				sysex = [0x7E][sysex_byte][data][0xF7]
				sysex = [0x7F][sysex_byte][data][0xF7] (after sending it will wait for reply)

				mod_byte = 0x70-0x7F
					0x70: extended sysex (ie: next byte won't be one of the protocol defined sysex'es)
					0x7E: do not require an immediate response from the receiving device.
					0x7F: prompt device to respond and to do so in real time.

				sysex_byte = 0x00-0x6F

Example common event:
			[0xFF][0x00][0x00][0xFF][0xFF][0xFF] (zero delay from previous message, reset system)
Example sysex event:
			[0xFF][0x00][0x01][0xF0][0x7E][0x07][0x01][0xF7] (1 tick after previous message, send string "1")
*/

// protocol version number (starting from v3.0 because I started from Firmata v2.6.0)
#define PROTOCOL_VERSION_MAJOR  3 // for non-compatible changes
#define PROTOCOL_VERSION_MINOR  0 // for backwards compatible changes

// max number of data bytes in incoming messages
#define PROTOCOL_MAX_EVENT_BYTES   64
#define PROTOCOL_TICK_BYTES	2 // the number of bytes to describe delta time

#define PROTOCOL_ENCODING_REPORT	0
// each input byte become 2*byte7bit to transmit (ie: bandwidth intensive)
#define PROTOCOL_ENCODING_NORMAL	1
// shift input bits to reduce the amount of output bytes to transmit (ie: CPU intensive)
#define PROTOCOL_ENCODING_COMPAT	2

// Realtime control signals
#define REALTIME_SIG_SYN			1 // start of synchronous operation
#define REALTIME_SIG_ACK			2 // confirm
#define REALTIME_SIG_NACK			3 // problem
#define REALTIME_SIG_FIN			4 // end of synchronous operation
#define REALTIME_SIG_RST			5 // reset session

// 1xxxxxxx:	status byte using control messages (128-255/0x80-0xFF)
//					0x80-0xEF: action message, specific to a port/pin and directly affect input/output
//					0xF0-0xF7: common message, do not require an immediate response from the receiving
//					0xF8-0xFF: realtime message, prompt device to respond and to do so in real time.
#define STATUS_PIN_MODE				0x80 // set a pin to INPUT/OUTPUT/PWM/etc
#define STATUS_DIGITAL_PORT_DATA	0x90 // data for a digital port (ie: 8 pins)
#define STATUS_DIGITAL_PIN_DATA		0xA0 // data for a digital pin
#define STATUS_ANALOG_PIN_DATA		0xB0 // data for an analog pin (or PWM)
#define STATUS_DIGITAL_PORT_REPORT	0xC0 // enable digital input by port pair
#define STATUS_DIGITAL_PIN_REPORT	0xD0 // enable digital input by pin #
#define STATUS_ANALOG_PIN_REPORT	0xE0 // enable analog input by pin #
#define STATUS_SYSEX_START			0xF0 // start a MIDI Sysex message
#define STATUS_MICROSTAMP_REPORT	0xF1 // report number of seconds since boot (or overflow)
#define STATUS_TIMESTAMP_REPORT		0xF2 // report number of seconds since boot (or overflow)
#define STATUS_CONGESTION_REPORT	0xF3 // report lag
#define STATUS_VERSION_REPORT		0xF4 // report protocol version
#define STATUS_INTERRUPT			0xF5 // interrupt current event (ex: signal error)
#define STATUS_ENCODING_SWITCH		0xF6 //
#define STATUS_SYSEX_END			0xF7 // end a MIDI Sysex message
#define STATUS_EMERGENCY_STOP1		0xF8 // stop activity on pin group 1
#define STATUS_EMERGENCY_STOP2		0xF9 // stop activity on pin group 2
#define STATUS_EMERGENCY_STOP3		0xFA // stop activity on pin group 3
#define STATUS_EMERGENCY_STOP4		0xFB // stop activity on pin group 4
#define STATUS_SYSTEM_PAUSE			0xFC // system pause
#define STATUS_SYSTEM_RESUME		0xFD // system resume (from pause)
#define STATUS_SYSTEM_HALT			0xFE // system halt
#define STATUS_SYSTEM_RESET			0xFF // MIDI system reset

// 0xxxxxxx: sysex using data messages (0-127/0x00-0x7F)
// 0x00-0x1F: DATA
#define SYSEX_DIGITAL_PIN_DATA		0x00 // send any value to any pin
#define SYSEX_ANALOG_PIN_DATA		0x01 // send and specify the analog reference source and R/W resolution
#define SYSEX_SCHEDULER_DATA		0x02 // scheduler request (see related sub commands)
#define SYSEX_ONEWIRE_DATA			0x03 // 1WIRE communication (see related sub commands)
#define SYSEX_UART_DATA				0x04 // UART communication (see related sub commands)
#define SYSEX_I2C_DATA				0x05 // I2C communication (see related sub commands)
#define SYSEX_SPI_DATA				0x06 // SPI communication (see related sub commands)
#define SYSEX_STRING_DATA			0x07 // encoded string (see related sub commands)
// 0x20-0x3F: REPORT
#define SYSEX_DIGITAL_PIN_REPORT	0x20 // report of ANY digital input pins
#define SYSEX_ANALOG_PIN_REPORT		0x21 // report of ANY analog input pins
#define SYSEX_VERSION_REPORT		0x22 // report firmware's version details (name, libs version, ...)
#define SYSEX_FEATURES_REPORT		0x23 // report features supported by the firmware
// 0x40-0x5F: REQUEST/REPLY
#define SYSEX_PINCAPS_REQ			0x40 // ask for supported modes and resolution of all pins
#define SYSEX_PINCAPS_REP			0x41 // reply with supported modes and resolution
#define SYSEX_PINMAP_REQ			0x42 // ask for mapping of analog to pin numbers
#define SYSEX_PINMAP_REP			0x43 // reply with mapping info
#define SYSEX_PINSTATE_REQ			0x44 // ask for a pin's current mode and value
#define SYSEX_PINSTATE_REP			0x45 // reply with pin's current mode and value
#define SYSEX_DEVICE_REQ			0x46 // PROPOSED: Generic Device Driver RPC
#define SYSEX_DEVICE_REP			0x47 // PROPOSED: Generic Device Driver RPC
#define SYSEX_RCSWITCH_IN			0x48 // PROPOSED: bridge to RCSwitch Arduino library
#define SYSEX_RCSWITCH_OUT			0x49 // PROPOSED: bridge to RCSwitch Arduino library
// 0x60-0x6F reserved for user-defined commands
// 0x70-0x7F sysex modifier
#define SYSEX_MOD_EXTEND			0x70 // sysex extension, the next 2 bytes define the extended ID
#define SYSEX_MOD_NON_REALTIME		0x7E // MIDI Reserved for non-realtime messages
#define SYSEX_MOD_REALTIME			0x7F // MIDI Reserved for realtime messages

// Digital data sub (0x00-0x7F)
#define SYSEX_SUB_DIGITAL_DATA_	0

// Analog data sub (0x00-0x7F)
#define SYSEX_SUB_ANALOG_DATA_	0

/* Scheduler data sub (0x00-0x7F)
The idea is to store a stream of messages on a microcontroller which is replayed later (either once or repeated).
A task is created by sending a create_task message.
The time-to-run is initialized with 0 (which means the task is not yet ready to run).
After filling up the taskdata with messages (using add_to_task command messages) a final schedule_task request is sent, that sets the time-to-run (in milliseconds after 'now').
If a task itself contains delay_task or schedule_task-messages these cause the execution of the task to pause and resume after the amount of time given in such message has elapsed.
If the last message in a task is a delay_task message the task is scheduled for reexecution after the amount of time specified.
If there's no delay_task message at the end of the task (so the time-to-run is not updated during the run) the task gets deleted after execution.
*/
#define SYSEX_SUB_SCHED_CREATE		0
#define SYSEX_SUB_SCHED_DELETE		1
#define SYSEX_SUB_SCHED_ADD			2
#define SYSEX_SUB_SCHED_DELAY		3
#define SYSEX_SUB_SCHED_SCHEDULE	4
#define SYSEX_SUB_SCHED_LIST_REQ	5
#define SYSEX_SUB_SCHED_LIST_REP	6
#define SYSEX_SUB_SCHED_TASK_REQ	7
#define SYSEX_SUB_SCHED_TASK_REP	8
#define SYSEX_SUB_SCHED_ERROR_REP	9
#define SYSEX_SUB_SCHED_RESET		127

// Onewire data sub (0x00-0x7F)
#define SYSEX_SUB_ONEWIRE_	0

// UART data sub (0x00-0x7F)
#define SYSEX_SUB_UART_		0

// I2C data sub (0x00-0x7F)
#define SYSEX_SUB_I2C_		0

// SPI data sub (0x00-0x7F)
#define SYSEX_SUB_SPI_		0

// string data sub (0x00-0x7F)
#define SYSEX_SUB_STRING_	0


typedef struct task_s {
	uint8_t id; // only 7bits used -> supports 127 tasks
	long ms; // ms to task
	int len;
	int pos;
	uint8_t *messages;
	struct task_s *next;
} task_t;

// callback function types
typedef void (*cbf_varg_t)(const char *format, ...);
typedef void (*cbf_data_t)(uint8_t argc, uint8_t *argv);
typedef uint8_t (*cbf_eval_t)(uint8_t count);
typedef void (*cbf_coder_t)(uint8_t *input, uint8_t count, uint8_t *output);

// callback functions
extern cbf_varg_t printString;
extern cbf_varg_t printErr;
extern cbf_eval_t cbEvalEnc;
extern cbf_coder_t cbEncoder;
extern cbf_eval_t cbEvalDec;
extern cbf_coder_t cbDecoder;

// events
extern uint16_t deltaTime; // ticks since last complete event
extern uint8_t eventBuffer[PROTOCOL_MAX_EVENT_BYTES]; // current event in transit
extern uint8_t eventSize; // current event size

// reset receive buffer
void bufferReset(void);

// copy buffer to event, returns the event size
uint8_t bufferCopy(uint8_t *event);

//
uint8_t encodingSwitch(uint8_t proto);

// receive 1 byte in default buffer (or given buffer)
// - uncomplete event: returns 0,
// - complete event: returns the number of stored bytes
uint8_t decodeEvent(uint8_t *byte, uint8_t size, uint8_t *event);

// prepare data for sending and write it at &event:
//		- for simple events argc=byte1 and argv=&byteN
//		- for sysex events = CMD_START_SYSEX, argv[0] is sysex modifier, and argv[1] sysex event, then data
// returns the event's number of bytes (to be eventually sent)
uint8_t encodeEvent(uint8_t cmd, uint8_t argc, uint8_t *argv, uint8_t *event);
// encode subs (task, ...)
uint8_t encodeTask(task_t *task, uint8_t error, uint8_t *event);

// print event in buffer (or given event)
void printEvent(uint8_t count, uint8_t *event);

#ifdef __cplusplus
}
#endif

#endif

