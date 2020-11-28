#ifndef LIBKNP_H
#define LIBKNP_H

#ifdef __GIT_REVPARSE__
#define RELEASE_LIBKNP __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <hal.h>
#include <protocol.h>

#define TICKTIME 1000

	extern char fwname[];
	extern char fwver[];

	// tasks
	extern task_t *tasks;

	// init once before looping run()
	void init(char *name, char *ver);

	// send a string to stdout
	void localLog(const char *format, ...);
	// send a string to the other side of the connection
	void remoteLog(const char *format, ...);

	// receive 1 event
	void getEvent(void);
	// mcu side: run 1 event
	void runEvent(uint8_t size, uint8_t *event);
	// run events from scheduler (if any), at the right time
	void runEventSched(uint16_t now);
	// print event in buffer (or given event)
	void printEvent(uint8_t size, uint8_t *event, char *output);

	// run 1 tick
	uint8_t run(void);

	//
	// commands to trigger events on the other side of the connection
	//

	// simple events
	void cmdPinMode(uint8_t pin, uint8_t mode);				// set mode (INPUT/OUTPUT/PWM/...) on remote preferred pin (for any pin check sysex)
	void cmdGetDigitalPort(uint8_t port, uint8_t timeout);	// get value on remote port
	void cmdSetDigitalPort(uint8_t port, uint8_t value);	// set value on remote port
	void cmdGetDigitalPin(uint8_t port, uint8_t timeout);	// get value on remote preferred pin
	void cmdSetDigitalPin(uint8_t pin, uint16_t value);		// set value on remote preferred pin
	void cmdGetAnalogPin(uint8_t pin, uint8_t timeout);		// get value on remote preferred pin
	void cmdSetAnalogPin(uint8_t pin, uint16_t value);		// set value on remote preferred pin
	void cmdHandshakeProtocolVersion(void);			// get/report the protocol version (for firmware version check the sysex)
	void cmdHandshakeEncoding(uint8_t proto);		// get/set/report encoding (7bit, 7bit compat, ...)
	void cmdGetInfo(uint8_t info);					// get one of the available infos (microstamp, timestamp, ...)
	void cmdSendSignal(uint8_t sig, uint8_t value);	// send 1-byte signal
	void cmdSendInterrupt(uint8_t irq, uint8_t value);	// send priority 1-byte signal (ie: interrupt current activity and handle this)
	void cmdEmergencyStop(uint8_t group);	// stop any activity on pin group
	void cmdSystemPause(uint16_t delay);	// set remote system pause, wait for delay before pause
	void cmdSystemResume(uint16_t time);	// set remote system resume (from emergency stop or from pause), wait for time before resume
	void cmdSystemReset(uint8_t mode);		// initialize a known default state: reset pins, delete any task, ...

	// sysex events & subs
	void cmdSysexPrefPins(uint8_t cmd, uint8_t pin);					// get/set preferred pins
	void cmdSysexPinGroups(uint8_t group, uint8_t cmd, uint8_t pin);	// get/set pin groups
	void cmdSysexDigitalPin(void);		// get/set any value on any pin
	void cmdSysexAnalogPin(void);		// get/set any value on any pin, and specify the analog reference source and R/W resolution
	void cmdSysexOneWireData(void);		// 1WIRE communication (see related sub commands)
	void cmdSysexUartData(void);		// UART communication (see related sub commands)
	void cmdSysexI2CData(void);			// I2C communication (see related sub commands)
	void cmdSysexSPIData(void);			// SPI communication (see related sub commands)
	void cmdSysexStringData(uint8_t len, char *string);// encoded string (see related sub commands)
	void cmdSysexScheduler(void);						// scheduler request (see related sub commands)
	void cmdSysexSchedCreate(uint8_t id, uint8_t len);	// scheduler create task
	void cmdSysexSchedDelete(uint8_t id);				// scheduler delete task
	void cmdSysexSchedAdd(uint8_t id, uint8_t size, uint8_t *event);// scheduler add event to task
	void cmdSysexSchedSchedule(uint8_t id, uint16_t delay);			// scheduler schedule
	void cmdSysexSchedDelay(uint16_t delay);			// scheduler delay task
	void cmdSysexSchedQueryList(void);					// scheduler query task list
	void cmdSysexSchedQueryTask(uint8_t id);			// scheduler query specific task
	void cmdSysexSchedReset(void);						// scheduler delete all tasks
	void cmdSysexVersion(uint8_t item); // report firmware version details
	void cmdSysexFeatures(void);		// report features supported by the firmware
	void cmdSysexPinCapsReq(void);		// ask for supported modes and resolution of all pins
	void cmdSysexPinCapsRep(void);		// reply with supported modes and resolution
	void cmdSysexPinMapReq(void);		// ask for mapping of analog to pin numbers
	void cmdSysexPinMapRep(void);		// reply with mapping info
	void cmdSysexPinStateReq(void);		// ask for a pin's current mode and value
	void cmdSysexPinStateRep(void);		// reply with pin's current mode and value
	void cmdSysexDeviceReq(void);		// Generic Device Driver RPC
	void cmdSysexDeviceRep(void);		// Generic Device Driver RPC
	void cmdSysexRCSwitchIn(void);		// bridge to RCSwitch Arduino library
	void cmdSysexRCSwitchOut(void);		// bridge to RCSwitch Arduino library


	//
	// events exec
	//

	// simple events
	void eventPinMode(uint8_t pin, uint8_t mode);
	void eventReportDigitalPort(uint8_t port, uint8_t timeout);
	void eventSetDigitalPort(uint8_t port, uint8_t value);
	void eventReportDigitalPin(uint8_t pin, uint8_t timeout);
	void eventSetDigitalPin(uint8_t pin, uint16_t value);
	void eventReportAnalogPin(uint8_t pin, uint8_t timeout);
	void eventSetAnalogPin(uint8_t pin, uint16_t value);
	void eventHandshakeProtocolVersion(void);
	void eventHandshakeEncoding(uint8_t proto);
	void eventReportInfo(uint8_t info, uint8_t value);
	void eventHandleSignal(uint8_t sig, uint8_t key, uint8_t value);
	void eventHandleInterrupt(uint8_t irq, uint8_t key, uint8_t value);
	void eventEmergencyStop(uint8_t group);
	void eventSystemPause(uint16_t delay);
	void eventSystemResume(uint16_t time);
	void eventSystemReset(uint8_t mode);

	// sysex events
	void eventSysexPrefPins(void);
	void eventSysexPinGroups(void);
	void eventSysexDigitalPin(void);
	void eventSysexAnalogPin(void);
	void eventSysexOneWire(void);
	void eventSysexUart(void);
	void eventSysexI2C(void);
	void eventSysexSPI(void);
	void eventSysexString(uint8_t argc, char *argv);
	void eventSysexScheduler(void);
	void eventSysexSchedCreate(uint8_t id, uint8_t len);
	void eventSysexSchedDelete(uint8_t id);
	void eventSysexSchedAdd(uint8_t id, uint8_t additionalBytes, uint8_t *message);
	void eventSysexSchedSchedule(uint8_t id, uint16_t delay);
	void eventSysexSchedDelay(uint16_t delay);
	void eventSysexSchedQueryList(void);
	void eventSysexSchedQueryTask(uint8_t id);
	void eventSysexSchedReset(void);
	void eventSysexVersion(uint8_t item, char *ver);
	void eventSysexFeatures(uint8_t feature);
	void eventSysexPinCapsReq(void);
	void eventSysexPinCapsRep(void);
	void eventSysexPinMapReq(void);
	void eventSysexPinMapRep(void);
	void eventSysexPinStateReq(void);
	void eventSysexPinStateRep(void);
	void eventSysexDeviceReq(void);
	void eventSysexDeviceRep(void);
	void eventSysexRCSwitchIn(void);
	void eventSysexRCSwitchOut(void);

#ifdef __cplusplus
}
#endif

#endif

