#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef __GIT_REVPARSE__
#define RELEASE_PROTOCOL __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <utility/macros.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

/*
The protocol mimics the MIDI protocol. Each byte of the MIDI byte stream
starts with 1 (most significant bit) for control bytes and starts with 0 for data bytes.
The MIDI 'delta time' is replaced by a 'sequence id' (optional bytes).
The byte stream is framed in 'events':

	event = [id][message]

		id = [0xFD][sequence id] (7 bit event identifier)

		message = [status_byte][status_data] (new event definition)

Common events have max 2 bytes of data.
To extend the amount of data in each event, the status byte can be 0xFF to start a sysex.
Sysex have virtually unlimited bytes of data. But must limit the number of data bytes to hw capabilities.
A sysex is any data byte (ie: 7bit) in between 'sysex start' (0xFF) and 'sysex end' (0xFE):

			sysex = [mod_byte][sysex_byte][data]
				sysex = [0x70][extended_sysex_byte][data]
				sysex = [0x7E][sysex_byte][data] (async)
				sysex = [0x7F][sysex_byte][data] (sync: after sending it will wait for reply)

				mod_byte = 0x70-0x7F
					0x70: extended sysex (ie: next byte won't be one of the protocol defined sysex'es)
					0x7E: do not require an immediate response from the receiving device.
					0x7F: prompt device to respond and to do so in real time.

				sysex_byte = 0x00-0x6F

Example common event:

			[0xFD][0x00][0xFC][0xFC][0xFC] (first message in sequence, reset system)

Example sysex event:

			[0xFD][0x00][0xFF][0x7D][0x07][0x01][0xFE] (second message in sequence, send string "1")

Protocol users can customize it using both the 0xF5-0xF8 common event codes (for short&fast messages) and
the 'extended sysex' own codes. Just write your defines and helper methods in protocol_custom.h,
then plug your decoder in customHandler function pointer. Remember: sysex, extended sysex included,
can have data bytes only (ie: bytes values < 128).

At the end of every event a CRC8 byte might be applied on transmit and discarded on receive (optional).
CRC, checking for control/data bits in each received byte, and a few more checks, should make the protocol robust enough.
If debug is enabled, errors are counted in uint8_t protocolErrorNo.
An increasing number of errors reveal electric interference and protocol version mismatch.
Errors are detailed in protocolErrors to improve diagnostics.
*/

// protocol version number (starting from v3.0 because I started from Firmata v2.6.0)
#define PROTOCOL_VERSION_MAJOR  3 // for non-compatible changes
#define PROTOCOL_VERSION_MINOR  0 // for backwards compatible changes

// 1xxxxxxx:	status byte using control messages (128-255/0x80-0xFF)
//					0x80-0xEF: action message, specific to a port/pin and directly affect input/output
//					0xF0-0xF7: common message, do not require an immediate response from the receiving
//					0xF8-0xFF: realtime message, prompt device to respond and to do so in real time.
#define STATUS_PIN_MODE				0x80 // set preferred pin to INPUT/OUTPUT/PWM/...
#define STATUS_DIGITAL_PORT_REPORT	0x90 // digital port value get
#define STATUS_DIGITAL_PORT_SET		0xA0 // digital port value set
#define STATUS_DIGITAL_PIN_REPORT	0xB0 // digital pin value request
#define STATUS_DIGITAL_PIN_SET		0xC0 // digital pin value report
#define STATUS_ANALOG_PIN_REPORT	0xD0 // analog pin value request
#define STATUS_ANALOG_PIN_SET		0xE0 // analog pin value report
#define STATUS_PROTOCOL_VERSION		0xF0 // protocol version
#define STATUS_PROTOCOL_ENCODING	0xF1 // protocol encoding
#define STATUS_INFO					0xF2 // 1-byte on-demand info
#define STATUS_SIGNAL				0xF3 // 1-byte 1-way signal
#define STATUS_INTERRUPT			0xF4 // 1-byte 1-way priority signal (ie: interrupt)
#define STATUS_CUSTOM_F5			0xF5 // custom defined
#define STATUS_CUSTOM_F6			0xF6 // custom defined
#define STATUS_CUSTOM_F7			0xF7 // custom defined
#define STATUS_CUSTOM_F8			0xF8 // custom defined
#define STATUS_EMERGENCY_STOP		0xF9 // stop activity on pin group X
#define STATUS_SYSTEM_PAUSE			0xFA // system pause
#define STATUS_SYSTEM_RESUME		0xFB // system resume (from pause)
#define STATUS_SYSTEM_RESET			0xFC // system reset
#define STATUS_EVENT_BEGIN			0xFD // event begin
#define STATUS_SYSEX_END			0xFE // end of sysex data
#define STATUS_SYSEX_START			0xFF // start of sysex data

// protocol error codes
#define PROTOCOL_ERR_UNKNOWN	0 //
#define PROTOCOL_ERR_CRC		1 // bad CRC
#define PROTOCOL_ERR_SIZE		2 // misplaced byte
#define PROTOCOL_ERR_NEED_DATA	3 // expected data, got control
#define PROTOCOL_ERR_NEED_CTRL	4 // expected ctrl, got data
#define PROTOCOL_ERR_START		5 // expected start, got something else
#define PROTOCOL_ERR_EVENT_UNKNOWN 6 //
#define PROTOCOL_ERR_TOTAL		7 //

// protocol encoding codes
#define PROTOCOL_ENCODING_UNKNOWN	0
#define PROTOCOL_ENCODING_NORMAL	1
#define PROTOCOL_ENCODING_COMPAT	2
#define PROTOCOL_ENCODING_REPORT	127

// 1-byte infos codes
//#define INFO_	0

// 1-byte signal codes
#define SIG_JITTER	0 // last tick finished in late (ie: developer is underachieving / MCU is too tiny / too much work)
#define SIG_DISCARD	1 // received bytes discarded (ie: protocol error)

// 0xxxxxxx: sysex using data messages (0-127/0x00-0x7F)
// 0x00-0x1F: DATA
#define SYSEX_PREFERRED_PINS_DATA	0x00 // get/set preferred pins
#define SYSEX_PINGROUPS_DATA		0x01 // get/set pin groups
#define SYSEX_DIGITAL_PIN_DATA		0x02 // get/set any value to any pin
#define SYSEX_ANALOG_PIN_DATA		0x03 // get/set and specify the analog reference source and R/W resolution
#define SYSEX_ONEWIRE_DATA			0x04 // 1WIRE communication (see related sub commands)
#define SYSEX_UART_DATA				0x05 // UART communication (see related sub commands)
#define SYSEX_I2C_DATA				0x06 // I2C communication (see related sub commands)
#define SYSEX_SPI_DATA				0x07 // SPI communication (see related sub commands)
#define SYSEX_STRING_DATA			0x08 // encoded string (see related sub commands)
#define SYSEX_SCHEDULER_DATA		0x09 // scheduler request (see related sub commands)
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
#define SYSEX_DEVICE_REQ			0x46 // generic device driver RPC request
#define SYSEX_DEVICE_REP			0x47 // generic device driver RPC reply
#define SYSEX_RCSWITCH_IN			0x48 // bridge to RCSwitch Arduino library
#define SYSEX_RCSWITCH_OUT			0x49 // bridge to RCSwitch Arduino library
// 0x60-0x6F reserved for later use
// 0x70-0x7F sysex modifier
#define SYSEX_MOD_ASYNC				0x7D // asyncronous message: it doesn't wait for the other side to answer
#define SYSEX_MOD_SYNC				0x7E // sync message: it waits for the other side to answer
#define SYSEX_MOD_EXTEND			0x7F // sysex extension, the next byte is the extended sysex ID

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

// version data sub
#define SYSEX_SUB_VERSION_FIRMWARE_NAME	0
#define SYSEX_SUB_VERSION_FIRMWARE_VER	1
#define SYSEX_SUB_VERSION_LIBKNP	2
#define SYSEX_SUB_VERSION_PROTOCOL	3
#define SYSEX_SUB_VERSION_HAL		4
#define SYSEX_SUB_VERSION_BOARD		5
#define SYSEX_SUB_VERSION_ARCH		6
#define SYSEX_SUB_VERSION_DEFINES	7
#define SYSEX_SUB_VERSION_ALL		127

// features data sub
#define SYSEX_SUB_FEATURES_ALL		127

// max number of data bytes in incoming messages
#define PROTOCOL_MAX_EVENT_BYTES   64
// use sequenceId TODO, make it optional
#define PROTOCOL_USE_SEQUENCEID	1
// use CRC8 TODO, make it optional
#define PROTOCOL_USE_CRC		1


/*
 * Preferred pins: events 0x80-0xE0 can point to 16 ports/pins only. Instead of using 'the first 16 pins'
 * the event code incorporates (as 'channel' in MIDI speaking) the index of the preferred pin array.
 * The host can configure arbitrary pins as 'preferred', and then use fast events (6 bytes+CRC) to manipulate
 * those. In order to manipulate other pins a sysex must be used, requiring more bytes to produce an event.
 */
// preferred pins (array index is the 'channel' in STATUS_XXX events)
extern uint8_t pins[16];
// pin groups (4 groups of 16 pins each, for STATUS_)
extern uint8_t group[4][16];

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
typedef void (*cbf_signal_t)(uint8_t sig, uint8_t value);

// callback functions
extern cbf_eval_t cbEvalEnc;
extern cbf_coder_t cbEncoder;
extern cbf_eval_t cbEvalDec;
extern cbf_coder_t cbDecoder;
extern cbf_data_t customHandler;
extern cbf_signal_t protocolSignal;

// events
extern uint8_t sequenceId; // unique event identifier
extern uint8_t eventBuffer[PROTOCOL_MAX_EVENT_BYTES]; // current event in transit
extern uint8_t eventSize; // current event size
extern uint8_t protocolDebug; // enable protocol debug (runs slower and collect stats)
extern uint8_t protocolErrors[PROTOCOL_ERR_TOTAL]; // error stats collector
extern uint8_t protocolErrorNo; // discarded events/bytes (flow errors)

// reset receive buffer
void bufferReset(void);
// copy buffer to store, returns the event size
uint8_t bufferStore(uint8_t *store);
// copy event to buffer
void bufferLoad(uint8_t *store, uint8_t size);


// get/set encoding
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
uint8_t encodePinMode(uint8_t *result, uint8_t pin, uint8_t mode);
uint8_t encodeReportDigitalPort(uint8_t *result, uint8_t port);
uint8_t encodeSetDigitalPort(uint8_t *result, uint8_t port, uint8_t value);
uint8_t encodeReportDigitalPin(uint8_t *result, uint8_t pin);
uint8_t encodeSetDigitalPin(uint8_t *result, uint8_t pin, uint8_t value);
uint8_t encodeReportAnalogPin(uint8_t *result, uint8_t pin);
uint8_t encodeSetAnalogPin(uint8_t *result, uint8_t pin, uint8_t value);
uint8_t encodeProtocolVersion(uint8_t *result);
uint8_t encodeProtocolEncoding(uint8_t *result, uint8_t proto);
uint8_t encodeInfo(uint8_t *result, uint8_t info, uint8_t value);
uint8_t encodeSignal(uint8_t *result, uint8_t key, uint8_t value);
uint8_t encodeInterrupt(uint8_t *result, uint8_t key, uint8_t value);
uint8_t encodeEmergencyStop(uint8_t *result, uint8_t group);
uint8_t encodeSystemPause(uint8_t *result, uint16_t delay);
uint8_t encodeSystemResume(uint8_t *result, uint16_t time);
uint8_t encodeSystemReset(uint8_t *result, uint8_t mode);
uint8_t encodeSysex(uint8_t *result, uint8_t argc, uint8_t *argv);
// encode subs (task, ...)
uint8_t encodeTask(uint8_t *result, task_t *task, uint8_t error);

#include <protocol_custom.h>

#ifdef __cplusplus
}
#endif

#endif

