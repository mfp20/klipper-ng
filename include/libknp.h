#ifndef LIBKNP_H
#define LIBKNP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hal.h>
#include <protocol.h>

#ifdef __GIT_REVPARSE__
#define RELEASE_LIBKNP __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#define TICKRATE 1000

	extern char fwname[];
	extern char fwver[];

	// tasks
	extern task_t *tasks;
	extern task_t *running;

	// init once before looping run()
	void init(char *name, char *ver);

	// send a string to the other side of the connection
	void remoteLog(const char *format, ...);

	//
	// commands to trigger events on the other side of the connection
	//

	// simple events
	void cmdPinMode(uint8_t pin, uint8_t mode);
	void cmdDigitalPortData(uint8_t port, uint8_t value);
	void cmdDigitalPinData(uint8_t pin, uint8_t value);
	void cmdAnalogPinData(uint8_t pin, uint8_t value);
	void cmdDigitalPortReport(uint8_t port);
	void cmdDigitalPinReport(uint8_t port);
	void cmdAnalogPinReport(uint8_t pin);
	void cmdMicrostampReport(void); // report number of seconds since boot (or overflow)
	void cmdTimestampReport(void); // report number of seconds since boot (or overflow)
	void cmdCongestionReport(uint8_t lag); // report lag
	void cmdVersionReport(void); // report protocol version
	void cmdInterrupt(void); // send a predefined signal
	void cmdEncodingSwitch(uint8_t proto);
	void cmdEmergencyStop1(void); // stop activity on pin group 1
	void cmdEmergencyStop2(void); // stop activity on pin group 2
	void cmdEmergencyStop3(void); // stop activity on pin group 3
	void cmdEmergencyStop4(void); // stop activity on pin group 4
	void cmdSystemPause(void); // system pause
	void cmdSystemResume(void); // system resume (from pause)
	void cmdSystemHalt(void); // system halt
	void cmdSystemReset(void); // initialize a known default state

	// sysex events
	void cmdSysexDigitalPinData(void); // send any value to any pin
	void cmdSysexAnalogPinData(void); // send and specify the analog reference source and R/W resolution
	void cmdSysexSchedulerData(void); // scheduler request (see related sub commands)
	void cmdSysexOneWireData(void); // 1WIRE communication (see related sub commands)
	void cmdSysexUartData(void); // UART communication (see related sub commands)
	void cmdSysexI2CData(void); // I2C communication (see related sub commands)
	void cmdSysexSPIData(void); // SPI communication (see related sub commands)
	void cmdSysexStringData(void); // encoded string (see related sub commands)
	void cmdSysexDigitalPinReport(void); // report of ANY digital input pins
	void cmdSysexAnalogPinReport(void); // report of ANY analog input pins
	void cmdSysexVersionReport(uint8_t item); // report firmware version details
	void cmdSysexFeaturesReport(void); // report features supported by the firmware
	void cmdSysexPinCapsReq(void); // ask for supported modes and resolution of all pins
	void cmdSysexPinCapsRep(void); // reply with supported modes and resolution
	void cmdSysexPinMapReq(void); // ask for mapping of analog to pin numbers
	void cmdSysexPinMapRep(void); // reply with mapping info
	void cmdSysexPinStateReq(void); // ask for a pin's current mode and value
	void cmdSysexPinStateRep(void); // reply with pin's current mode and value
	void cmdSysexDeviceReq(void); // Generic Device Driver RPC
	void cmdSysexDeviceRep(void); // Generic Device Driver RPC
	void cmdSysexRCSwitchIn(void); // bridge to RCSwitch Arduino library
	void cmdSysexRCSwitchOut(void); // bridge to RCSwitch Arduino library

	// sysex sub: scheduler events
	void cmdSchedCreate(uint8_t id, uint8_t len);
	void cmdSchedDelete(uint8_t id);
	void cmdSchedAdd(uint8_t id, uint8_t additionalBytes, uint8_t *message);
	void cmdSchedSchedule(uint8_t id, uint16_t delay);
	void cmdSchedDelay(uint16_t delay);
	void cmdSchedQueryList(void);
	void cmdSchedQueryTask(uint8_t id);
	void cmdSchedReset(void);

	//
	// events produced by commands received from the other side of the connection
	//

	// simple events
	void eventPinMode(uint8_t pin, uint8_t mode);
	void eventDigitalPortData(uint8_t port, uint8_t value);
	void eventDigitalPinData(uint8_t pin, uint8_t value);
	void eventAnalogPinData(uint8_t pin, uint8_t value);
	void eventDigitalPortReport(uint8_t port);
	void eventDigitalPinReport(uint8_t pin);
	void eventAnalogPinReport(uint8_t pin);
	void eventMicrostampReport(void); // report number of seconds since boot (or overflow)
	void eventTimestampReport(void); // report number of seconds since boot (or overflow)
	void eventCongestionReport(uint8_t lag); // report lag
	void eventVersionReport(void); // report protocol version
	void eventInterrupt(void); // interrupt current event (ex: signal error)
	void eventEncodingSwitch(uint8_t proto);
	void eventEmergencyStop1(void); // stop activity on pin group 1
	void eventEmergencyStop2(void); // stop activity on pin group 2
	void eventEmergencyStop3(void); // stop activity on pin group 3
	void eventEmergencyStop4(void); // stop activity on pin group 4
	void eventSystemPause(uint16_t delay); // system pause
	void eventSystemResume(uint16_t delay); // system resume (from pause)
	void eventSystemHalt(void); // system halt
	void eventSystemReset(void); // initialize a known default state

	// sysex events
	void eventSysexDigitalPinData(void); // send any value to any pin
	void eventSysexAnalogPinData(void); // send and specify the analog reference source and R/W resolution
	void eventSysexSchedulerData(void); // scheduler request (see related sub commands)
	void eventSysexOneWireData(void); // 1WIRE communication (see related sub commands)
	void eventSysexUartData(void); // UART communication (see related sub commands)
	void eventSysexI2CData(void); // I2C communication (see related sub commands)
	void eventSysexSPIData(void); // SPI communication (see related sub commands)
	void eventSysexStringData(uint8_t argc, char *argv); // encoded string (see related sub commands)
	void eventSysexDigitalPinReport(void); // report of ANY digital input pins
	void eventSysexAnalogPinReport(void); // report of ANY analog input pins
	void eventSysexVersionReport(uint8_t item, char *ver); // report firmware version details
	void eventSysexFeaturesReport(void); // report features supported by the firmware
	void eventSysexPinCapsReq(void); // ask for supported modes and resolution of all pins
	void eventSysexPinCapsRep(void); // reply with supported modes and resolution
	void eventSysexPinMapReq(void); // ask for mapping of analog to pin numbers
	void eventSysexPinMapRep(void); // reply with mapping info
	void eventSysexPinStateReq(void); // ask for a pin's current mode and value
	void eventSysexPinStateRep(void); // reply with pin's current mode and value
	void eventSysexDeviceReq(void); // Generic Device Driver RPC
	void eventSysexDeviceRep(void); // Generic Device Driver RPC
	void eventSysexRCSwitchIn(void); // bridge to RCSwitch Arduino library
	void eventSysexRCSwitchOut(void); // bridge to RCSwitch Arduino library

	// sysex sub: scheduler events
	void eventSchedCreate(uint8_t id, uint8_t len);
	void eventSchedDelete(uint8_t id);
	void eventSchedAdd(uint8_t id, uint8_t additionalBytes, uint8_t *message);
	void eventSchedSchedule(uint8_t id, uint16_t delay);
	void eventSchedDelay(uint16_t delay);
	void eventSchedQueryList(void);
	void eventSchedQueryTask(uint8_t id);
	void eventSchedReset(void);

	//
	// system
	//

	// run 1 event
	void runEvent(uint8_t size, uint8_t *event);
	// print event in buffer (or given event)
	void printEvent(uint8_t count, uint8_t *event, char *output);
	// run scheduled tasks (if any), at the right time
	void runScheduler(uint16_t now);
	// run 1 tick
	void run(void);

#ifdef __cplusplus
}
#endif

#endif

