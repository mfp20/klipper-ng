
#ifndef LIBKNP_H
#define LIBKNP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hal.h>
#include <protocol.h>

#define TICKRATE 1000

//
// DEVICE CODE
//

	void initDevice(void);
	void runDevice(void);
	void eventRunDevice(uint8_t size, uint8_t *event);

	// simple events
	void eventPinMode(uint8_t pin, uint8_t mode);
	void eventDigitalPortData(uint8_t port, uint8_t value);
	void eventDigitalPinData(uint8_t pin, uint8_t value);
	void eventAnalogPinData(uint8_t pin, uint8_t value);
	void eventDigitalPortReport(uint8_t port, uint8_t value);
	void eventDigitalPinReport(uint8_t port, uint8_t value);
	void eventAnalogPinReport(uint8_t pin, uint8_t value);
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
	void eventSystemPause(void); // system pause
	void eventSystemResume(void); // system resume (from pause)
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
	void eventSysexStringData(void); // encoded string (see related sub commands)
	void eventSysexDigitalPinReport(void); // report of ANY digital input pins
	void eventSysexAnalogPinReport(void); // report of ANY analog input pins
	void eventSysexVersionReport(void); // report firmware version details
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
// HOST code
//

	void initHost(void);
	void runHost(void);
	void eventRunHost(uint8_t size, uint8_t *event);

	// typedefs for callbacks
	typedef void (*cbf_void_t)(void);
	typedef void (*cbf_short_t)(uint8_t);
	typedef void (*cbf_long_t)(uint16_t);
	typedef void (*cbf_ss_t)(uint8_t,uint8_t);
	typedef void (*cbf_sl_t)(uint8_t,uint16_t);
	typedef void (*cbf_sss_t)(uint8_t,uint8_t,uint8_t);

	// same names of event* functions; callbacks to be defined in host specific code
	extern cbf_ss_t cbPinMode;
	extern cbf_ss_t cbDigitalPortData;
	extern cbf_ss_t cbDigitalPinData;
	extern cbf_ss_t cbAnalogPinData;
	extern cbf_ss_t cbDigitalPortReport;
	extern cbf_ss_t cbDigitalPinReport;
	extern cbf_ss_t cbAnalogPinReport;
	extern cbf_void_t cbMicrostampReport;
	extern cbf_void_t cbTimestampReport;
	extern cbf_short_t cbCongestionReport;
	extern cbf_void_t cbVersionReport;
	extern cbf_void_t cbInterrupt;
	extern cbf_short_t cbEncodingSwitch;
	extern cbf_void_t cbEmergencyStop1;
	extern cbf_void_t cbEmergencyStop2;
	extern cbf_void_t cbEmergencyStop3;
	extern cbf_void_t cbEmergencyStop4;
	extern cbf_void_t cbSystemPause;
	extern cbf_void_t cbSystemResume;
	extern cbf_void_t cbSystemHalt;
	extern cbf_void_t cbSystemReset;

	// sysex events
	extern cbf_void_t cbSysexDigitalPinData;
	extern cbf_void_t cbSysexAnalogPinData;
	extern cbf_void_t cbSysexSchedulerData;
	extern cbf_void_t cbSysexOneWireData;
	extern cbf_void_t cbSysexUartData;
	extern cbf_void_t cbSysexI2CData;
	extern cbf_void_t cbSysexSPIData;
	extern cbf_void_t cbSysexStringData;
	extern cbf_void_t cbSysexDigitalPinReport;
	extern cbf_void_t cbSysexAnalogPinReport;
	extern cbf_void_t cbSysexVersionReport;
	extern cbf_void_t cbSysexFeaturesReport;
	extern cbf_void_t cbSysexPinCapsReq;
	extern cbf_void_t cbSysexPinCapsRep;
	extern cbf_void_t cbSysexPinMapReq;
	extern cbf_void_t cbSysexPinMapRep;
	extern cbf_void_t cbSysexPinStateReq;
	extern cbf_void_t cbSysexPinStateRep;
	extern cbf_void_t cbSysexDeviceReq;
	extern cbf_void_t cbSysexDeviceRep;
	extern cbf_void_t cbSysexRCSwitchIn;
	extern cbf_void_t cbSysexRCSwitchOut;

	// sysex sub: scheduler events
	extern cbf_ss_t cbSchedCreate;
	extern cbf_short_t cbSchedDelete;
	extern cbf_sss_t cbSchedAdd;
	extern cbf_sl_t cbSchedSchedule;
	extern cbf_long_t cbSchedDelay;
	extern cbf_void_t cbSchedQueryList;
	extern cbf_short_t cbSchedQueryTask;
	extern cbf_void_t cbSchedReset;

#ifdef __cplusplus
}
#endif

#endif

