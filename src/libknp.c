
#include <libknp.h>

uint16_t mstart = 0;
uint16_t ustart = 0;
uint16_t jitter = 0;
uint8_t txtData, binData, peerData;
uint8_t encodedEvent[PROTOCOL_MAX_DATA];

//
// DEVICE CODE
//

void initDevice(void) {
	// init hardware
	halInit();

	// setup protocol callbacks
	printString = logWrite;
	printErr = errWrite;
	encodingSwitch(PROTOCOL_ENCODING_NORMAL);
	runEvent = eventRunDevice;
}

void runDevice(void) {
	// --- start
	ustart = micros();
	mstart = millis();

	// --- evaluate performance and signal lag
	if (jitter>=TICKRATE) {
		eventCongestionReport(jitter/1000);
		jitter = jitter%1000;
	}

	// 1. run hardware tasks
	halRun();

	// 2. receive and run 1 event (arbitrary timeout 1-999 us)
	if (binConsole->available(binConsole)) {
		while(1) {
			binConsole->read(binConsole, &binData, 1, 1);
			if (decodeEvent(&binData, 0, NULL)) {
				runEvent(0, NULL);
				break;
			}
			if (mstart!=millis()) break;
		}
	}

	// 3. run scheduled tasks
	runScheduler(mstart);

	// --- evaluate spare time
	deltaTime = elapsed(mstart, ustart, micros(), millis());
	if (deltaTime >= TICKRATE) {
		jitter = jitter+(deltaTime-TICKRATE);
		return;
	}

	// --- spend spare time
	while(deltaTime<TICKRATE) {
		// receive and run 1 event (arbitrary timeout 1-999 us)
		if (binConsole->available(binConsole)) {
			while(1) {
				binConsole->read(binConsole, &binData, 1, 1);
				if (decodeEvent(&binData, 0, NULL)) {
					runEvent(0, NULL);
					break;
				}
				if (mstart!=millis()) break;
			}
		}
		// evaluate elapsed time
		deltaTime = elapsed(mstart, ustart, micros(), millis());
	}
	// evaluate jitter
	jitter = jitter+(deltaTime-TICKRATE);
}

void eventRunDevice(uint8_t size, uint8_t *event) {
	uint8_t datalen = 0;
	// TODO fix event indexes
	switch(event[0]) {
		case STATUS_PIN_MODE:
			eventPinMode(event[1], event[2]);
			break;
		case STATUS_DIGITAL_PORT_DATA:
			eventDigitalPortData(event[0] & 0x0F, (event[1] << 7) + event[2]);
			break;
		case STATUS_DIGITAL_PIN_DATA:
			eventDigitalPinData(event[1], event[2]);
			break;
		case STATUS_ANALOG_PIN_DATA:
			eventAnalogPinData(event[0] & 0x0F, (event[1] << 7) + event[2]);
			break;
		case STATUS_DIGITAL_PORT_REPORT:
			break;
		case STATUS_DIGITAL_PIN_REPORT:
			eventDigitalPinReport(event[0] & 0x0F, event[1]);
			break;
		case STATUS_ANALOG_PIN_REPORT:
			eventDigitalPinReport(event[0] & 0x0F, event[1]);
			break;
		case STATUS_MICROSTAMP_REPORT:
			eventMicrostampReport();
			break;
		case STATUS_TIMESTAMP_REPORT:
			eventTimestampReport();
			break;
		case STATUS_CONGESTION_REPORT:
			eventCongestionReport(event[12]);
			break;
		case STATUS_VERSION_REPORT:
			eventVersionReport();
			break;
		case STATUS_INTERRUPT:
			eventInterrupt();
			break;
		case STATUS_ENCODING_SWITCH:
			eventEncodingSwitch(event[32]);
			break;
		case STATUS_EMERGENCY_STOP1:
			eventEmergencyStop1();
			break;
		case STATUS_EMERGENCY_STOP2:
			eventEmergencyStop2();
			break;
		case STATUS_EMERGENCY_STOP3:
			eventEmergencyStop3();
			break;
		case STATUS_EMERGENCY_STOP4:
			eventEmergencyStop4();
			break;
		case STATUS_SYSTEM_PAUSE:
			eventSystemPause();
			break;
		case STATUS_SYSTEM_RESUME:
			eventSystemResume();
			break;
		case STATUS_SYSTEM_HALT:
			eventSystemHalt();
			break;
		case STATUS_SYSTEM_RESET:
			eventSystemReset();
			break;
		case STATUS_SYSEX_START:
			switch (event[1]) { // first byte is sysex start, second byte is command
				case SYSEX_MOD_EXTEND:
					switch(event[2]+event[3]) { // second byte is SYSEX_EXTEND, third&forth bytes are the extended commands
						default:
							errWrite("Not Implemented: %d (SYSEX_EXTEND)\n", event[1]);
					}
					break;
				case SYSEX_DIGITAL_PIN_DATA:
					eventSysexDigitalPinData();
					break;
				case SYSEX_ANALOG_PIN_DATA:
					eventSysexAnalogPinData();
					break;
				case SYSEX_SCHEDULER_DATA:
					if (size > 0) {
						switch (event[0]) {
							case SYSEX_SUB_SCHED_CREATE:
								if (size == 4) {
									eventSchedCreate(event[1], event[2] | event[3] << 7);
								}
								break;
							case SYSEX_SUB_SCHED_ADD:
								if (size > 2) {
									eventSchedAdd(event[1], size - 2, event + 2); // addToTask copies data...
								}
								break;
							case SYSEX_SUB_SCHED_DELAY:
								if (size == 6) {
									//event++; compiler error after moving the sysex switch
									eventSchedDelay(*(long*)((uint8_t*)event));
								}
								break;
							case SYSEX_SUB_SCHED_SCHEDULE:
								if (size == 7) { // one byte taskid, 5 bytes to encode 4 bytes of long
									eventSchedSchedule(event[1], *(long*)((uint8_t*)event + 2)); // event[1] | event[2]<<8 | event[3]<<16 | event[4]<<24
								}
								break;
							case SYSEX_SUB_SCHED_LIST_REQ:
								eventSchedQueryList();
								break;
							case SYSEX_SUB_SCHED_TASK_REQ:
								if (size == 2) {
									eventSchedQueryTask(event[1]);
								}
								break;
							case SYSEX_SUB_SCHED_DELETE:
								if (size == 2) {
									eventSchedDelete(event[1]);
								}
								break;
							case SYSEX_SUB_SCHED_RESET:
								eventSchedReset();
						}
					}
					break;
				case SYSEX_ONEWIRE_DATA:
					eventSysexOneWireData();
					break;
				case SYSEX_UART_DATA:
					eventSysexUartData();
					break;
				case SYSEX_I2C_DATA:
					eventSysexI2CData();
					break;
				case SYSEX_SPI_DATA:
					eventSysexSPIData();
					break;
				case SYSEX_STRING_DATA:
					// The string length will only be at most half the size of the
					// stored input buffer so we can decode the string within the buffer.
					datalen = (size-1)/2;
					cbDecoder(event, datalen, event);
					// Make sure string is null terminated. This may be the case for data
					// coming from client libraries in languages that don't null terminate
					// strings.
					if (event[datalen-1]!='\0') event[datalen] = '\0';
					logWrite((char *)event);
					break;
				case SYSEX_DIGITAL_PIN_REPORT:
					eventSysexDigitalPinReport();
					break;
				case SYSEX_ANALOG_PIN_REPORT:
					eventSysexAnalogPinReport();
					break;
				case SYSEX_VERSION_REPORT:
					eventSysexVersionReport();
					break;
				case SYSEX_FEATURES_REPORT:
					eventSysexFeaturesReport();
					break;
				case SYSEX_PINCAPS_REQ:
					eventSysexPinCapsReq();
					break;
				case SYSEX_PINCAPS_REP:
					eventSysexPinCapsRep();
					break;
				case SYSEX_PINMAP_REQ:
					eventSysexPinMapReq();
					break;
				case SYSEX_PINMAP_REP:
					eventSysexPinMapRep();
					break;
				case SYSEX_PINSTATE_REQ:
					eventSysexPinStateReq();
					break;
				case SYSEX_PINSTATE_REP:
					eventSysexPinStateRep();
					break;
				case SYSEX_DEVICE_REQ:
					eventSysexDeviceReq();
					break;
				case SYSEX_DEVICE_REP:
					eventSysexDeviceRep();
					break;
				case SYSEX_RCSWITCH_IN:
					eventSysexRCSwitchIn();
					break;
				case SYSEX_RCSWITCH_OUT:
					eventSysexRCSwitchOut();
					break;
				default:
					errWrite("Not Implemented: %d (Unknown command)\n", event[1]);
					break;
			}
			break;
		default:
			break;
	}
}

void eventPinMode(uint8_t pin, uint8_t mode) {
	errWrite("Not Implemented: \n");
}

void eventDigitalPortData(uint8_t port, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void eventDigitalPinData(uint8_t pin, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void eventAnalogPinData(uint8_t pin, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void eventDigitalPortReport(uint8_t port, uint8_t en) {
	errWrite("Not Implemented: \n");
}

void eventDigitalPinReport(uint8_t port, uint8_t en) {
	errWrite("Not Implemented: \n");
}

void eventAnalogPinReport(uint8_t pin, uint8_t en) {
	errWrite("Not Implemented: \n");
}

void eventMicrostampReport(void) {
	errWrite("Not Implemented: \n");
}

void eventTimestampReport(void) {
	errWrite("Not Implemented: \n");
}

void eventCongestionReport(uint8_t lag) {
	uint8_t evsize = encodeEvent(STATUS_CONGESTION_REPORT, lag, NULL, encodedEvent);
	binSend(evsize, encodedEvent);
}

void eventVersionReport(void) {
	uint8_t byte = PROTOCOL_VERSION_MINOR;
	uint8_t evsize = encodeEvent(STATUS_VERSION_REPORT, PROTOCOL_VERSION_MAJOR, &byte, encodedEvent);
	binSend(evsize, encodedEvent);
}

void eventInterrupt(void) {
	errWrite("Not Implemented: \n");
}

void eventEncodingSwitch(uint8_t proto) {
	if (proto==PROTOCOL_ENCODING_REPORT) {
		proto = encodingSwitch(proto);
		uint8_t evsize = encodeEvent(STATUS_ENCODING_SWITCH, proto, NULL, encodedEvent);
		binSend(evsize, encodedEvent);
	} else {
		encodingSwitch(proto);
	}
}

void eventEmergencyStop1(void) {
	errWrite("Not Implemented: \n");
}

void eventEmergencyStop2(void) {
	errWrite("Not Implemented: \n");
}

void eventEmergencyStop3(void) {
	errWrite("Not Implemented: \n");
}

void eventEmergencyStop4(void) {
	errWrite("Not Implemented: \n");
}

void eventSystemPause(void) {
	errWrite("Not Implemented: \n");
}

void eventSystemResume(void) {
	errWrite("Not Implemented: \n");
}

void eventSystemHalt(void) {
	errWrite("Not Implemented: \n");
}

void eventSystemReset(void) {
	// TODO set pins with analog capability to analog input
	// and set digital pins to digital output
	for (uint8_t i = 0; i < TOTAL_PINS; i++) {
		if (IS_PIN_ANALOG(i)) {
		} else if (IS_PIN_DIGITAL(i)) {
		}
	}
	// reset pin_status
	free(pin_status);
	pin_status_size = 0;
}

void eventSysexDigitalPinData(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexAnalogPinData(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexSchedulerData(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexOneWireData(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexUartData(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexI2CData(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexSPIData(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexStringData(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexDigitalPinReport(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexAnalogPinReport(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexVersionReport(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexFeaturesReport(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexPinCapsReq(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexPinCapsRep(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexPinMapReq(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexPinMapRep(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexPinStateReq(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexPinStateRep(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexDeviceReq(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexDeviceRep(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexRCSwitchIn(void) {
	errWrite("Not Implemented: \n");
}

void eventSysexRCSwitchOut(void) {
	errWrite("Not Implemented: \n");
}

static task_t *findTask(uint8_t id) {
	task_t *currentTask = tasks;
	while (currentTask) {
		if (id == currentTask->id) {
			return currentTask;
		} else {
			currentTask = currentTask->next;
		}
	}
	return NULL;
}

static void reportTask(task_t *task, bool error) {
	uint8_t evsize = encodeTask(task, error, encodedEvent);
	binSend(evsize, encodedEvent);
}

void eventSchedCreate(uint8_t id, uint8_t len) {
	task_t *existing = findTask(id);
	if (existing) {
		reportTask(existing, true);
	} else {
		task_t *newTask = (task_t*)malloc(sizeof(task_t) + len);
		newTask->id = id;
		newTask->ms = 0;
		newTask->len = len;
		newTask->next = tasks;
		newTask->pos = 0;
		newTask->messages = (uint8_t *)malloc(sizeof(uint8_t)*len);
		tasks = newTask;
	}
}

void eventSchedDelete(uint8_t id) {
	task_t *current = tasks;
	task_t *previous = NULL;
	while (current) {
		if (current->id == id) {
			if (previous) {
				previous->next = current->next;
			} else {
				tasks = current->next;
			}
			free(current->messages);
			free(current);
			return;
		} else {
			previous = current;
			current = current->next;
		}
	}
}

void eventSchedAdd(uint8_t id, uint8_t additionalBytes, uint8_t *message) {
	task_t *existing = findTask(id);
	if (existing) { //task exists and has not been fully loaded yet
		if (existing->pos + additionalBytes <= existing->len) {
			for (int i = 0; i < additionalBytes; i++) {
				existing->messages[existing->pos++] = message[i];
			}
		}
	} else {
		reportTask(NULL, true);
	}
}

void eventSchedSchedule(uint8_t id, uint16_t delay) {
	task_t *existing = findTask(id);
	if (existing) {
		existing->pos = 0;
		existing->ms = millis() + delay;
	} else {
		reportTask(NULL, true);
	}
}

void eventSchedDelay(uint16_t delay_ms) {
	if (running) {
		long now = millis();
		running->ms += delay_ms;
		if (running->ms < now) { // if delay time allready passed by schedule to 'now'.
			running->ms = now;
		}
	}
}

void eventSchedQueryList(void) {
	uint8_t byte = STATUS_SYSEX_START;
	binConsole->write(binConsole, &byte, 1, 0);
	byte = SYSEX_SCHEDULER_DATA;
	binConsole->write(binConsole, &byte, 1, 0);
	byte = SYSEX_SUB_SCHED_LIST_REP;
	binConsole->write(binConsole, &byte, 1, 0);
	task_t *task = tasks;
	while (task) {
		binConsole->write(binConsole, &task->id, 1, 0);
		task = task->next;
	}
	byte = STATUS_SYSEX_END;
	binConsole->write(binConsole, &byte, 1, 0);
}

void eventSchedQueryTask(uint8_t id) {
	task_t *task = findTask(id);
	reportTask(task, false);
}

void eventSchedReset(void) {
	while (tasks) {
		task_t *next = tasks->next;
		free(tasks);
		tasks = next;
	}
}

//
// HOST code
//

void initHost(void) {
	// init hardware
	halInit();

	// setup protocol callbacks
	printString = logWrite;
	printErr = errWrite;
	encodingSwitch(PROTOCOL_ENCODING_NORMAL);
	runEvent = eventRunHost;
}

void runHost(void) {
	runDevice();
}

void eventRunHost(uint8_t size, uint8_t *event) {
	uint8_t datalen = 0;
	// TODO adjust event indexes to consider deltaTime (ie: +3 offset)
	switch(event[0]) {
		case STATUS_PIN_MODE:
			cbPinMode(event[1], event[2]);
			break;
		case STATUS_DIGITAL_PORT_DATA:
			cbDigitalPortData(event[0] & 0x0F, (event[1] << 7) + event[2]);
			break;
		case STATUS_DIGITAL_PIN_DATA:
			cbDigitalPinData(event[1], event[2]);
			break;
		case STATUS_ANALOG_PIN_DATA:
			cbAnalogPinData(event[0] & 0x0F, (event[1] << 7) + event[2]);
			break;
		case STATUS_DIGITAL_PORT_REPORT:
			break;
		case STATUS_DIGITAL_PIN_REPORT:
			cbDigitalPinReport(event[0] & 0x0F, event[1]);
			break;
		case STATUS_ANALOG_PIN_REPORT:
			cbDigitalPinReport(event[0] & 0x0F, event[1]);
			break;
		case STATUS_MICROSTAMP_REPORT:
			cbMicrostampReport();
			break;
		case STATUS_TIMESTAMP_REPORT:
			cbTimestampReport();
			break;
		case STATUS_CONGESTION_REPORT:
			cbCongestionReport(event[12]);
			break;
		case STATUS_VERSION_REPORT:
			cbVersionReport();
			break;
		case STATUS_INTERRUPT:
			cbInterrupt();
			break;
		case STATUS_ENCODING_SWITCH:
			cbEncodingSwitch(event[32]);
			break;
		case STATUS_EMERGENCY_STOP1:
			cbEmergencyStop1();
			break;
		case STATUS_EMERGENCY_STOP2:
			cbEmergencyStop2();
			break;
		case STATUS_EMERGENCY_STOP3:
			cbEmergencyStop3();
			break;
		case STATUS_EMERGENCY_STOP4:
			cbEmergencyStop4();
			break;
		case STATUS_SYSTEM_PAUSE:
			cbSystemPause();
			break;
		case STATUS_SYSTEM_RESUME:
			cbSystemResume();
			break;
		case STATUS_SYSTEM_HALT:
			cbSystemHalt();
			break;
		case STATUS_SYSTEM_RESET:
			cbSystemReset();
			break;
		case STATUS_SYSEX_START:
			switch (event[1]) { // first byte is sysex start, second byte is command
				case SYSEX_MOD_EXTEND:
					switch(event[2]+event[3]) { // second byte is SYSEX_EXTEND, third&forth bytes are the extended commands
						default:
							errWrite("Not Implemented: %d (SYSEX_EXTEND)\n", event[1]);
					}
					break;
				case SYSEX_DIGITAL_PIN_DATA:
					cbSysexDigitalPinData();
					break;
				case SYSEX_ANALOG_PIN_DATA:
					cbSysexAnalogPinData();
					break;
				case SYSEX_SCHEDULER_DATA:
					if (size > 0) {
						switch (event[0]) {
							case SYSEX_SUB_SCHED_CREATE:
								if (size == 4) {
									cbSchedCreate(event[1], event[2] | event[3] << 7);
								}
								break;
							case SYSEX_SUB_SCHED_ADD:
								if (size > 2) {
									cbSchedAdd(event[1], size - 2, event[2]); // addToTask copies data...
								}
								break;
							case SYSEX_SUB_SCHED_DELAY:
								if (size == 6) {
									//event++; compiler error after moving the sysex switch
									cbSchedDelay(*(long*)((uint8_t*)event));
								}
								break;
							case SYSEX_SUB_SCHED_SCHEDULE:
								if (size == 7) { // one byte taskid, 5 bytes to encode 4 bytes of long
									cbSchedSchedule(event[1], *(long*)((uint8_t*)event + 2)); // event[1] | event[2]<<8 | event[3]<<16 | event[4]<<24
								}
								break;
							case SYSEX_SUB_SCHED_LIST_REQ:
								cbSchedQueryList();
								break;
							case SYSEX_SUB_SCHED_TASK_REQ:
								if (size == 2) {
									cbSchedQueryTask(event[1]);
								}
								break;
							case SYSEX_SUB_SCHED_DELETE:
								if (size == 2) {
									cbSchedDelete(event[1]);
								}
								break;
							case SYSEX_SUB_SCHED_RESET:
								cbSchedReset();
						}
					}
					break;
				case SYSEX_ONEWIRE_DATA:
					cbSysexOneWireData();
					break;
				case SYSEX_UART_DATA:
					cbSysexUartData();
					break;
				case SYSEX_I2C_DATA:
					cbSysexI2CData();
					break;
				case SYSEX_SPI_DATA:
					cbSysexSPIData();
					break;
				case SYSEX_STRING_DATA:
					// The string length will only be at most half the size of the
					// stored input buffer so we can decode the string within the buffer.
					datalen = (size-1)/2;
					cbDecoder(event, datalen, event);
					// Make sure string is null terminated. This may be the case for data
					// coming from client libraries in languages that don't null terminate
					// strings.
					if (event[datalen-1]!='\0') event[datalen] = '\0';
					logWrite((char *)event);
					break;
				case SYSEX_DIGITAL_PIN_REPORT:
					cbSysexDigitalPinReport();
					break;
				case SYSEX_ANALOG_PIN_REPORT:
					cbSysexAnalogPinReport();
					break;
				case SYSEX_VERSION_REPORT:
					cbSysexVersionReport();
					break;
				case SYSEX_FEATURES_REPORT:
					cbSysexFeaturesReport();
					break;
				case SYSEX_PINCAPS_REQ:
					cbSysexPinCapsReq();
					break;
				case SYSEX_PINCAPS_REP:
					cbSysexPinCapsRep();
					break;
				case SYSEX_PINMAP_REQ:
					cbSysexPinMapReq();
					break;
				case SYSEX_PINMAP_REP:
					cbSysexPinMapRep();
					break;
				case SYSEX_PINSTATE_REQ:
					cbSysexPinStateReq();
					break;
				case SYSEX_PINSTATE_REP:
					cbSysexPinStateRep();
					break;
				case SYSEX_DEVICE_REQ:
					cbSysexDeviceReq();
					break;
				case SYSEX_DEVICE_REP:
					cbSysexDeviceRep();
					break;
				case SYSEX_RCSWITCH_IN:
					cbSysexRCSwitchIn();
					break;
				case SYSEX_RCSWITCH_OUT:
					cbSysexRCSwitchOut();
					break;
				default:
					errWrite("Not Implemented: %d (Unknown command)\n", event[1]);
					break;
			}
			break;
		default:
			break;
	}
}

cbf_ss_t cbPinMode = NULL;
cbf_ss_t cbDigitalPortData = NULL;
cbf_ss_t cbDigitalPinData = NULL;
cbf_ss_t cbAnalogPinData = NULL;
cbf_ss_t cbDigitalPortReport = NULL;
cbf_ss_t cbDigitalPinReport = NULL;
cbf_ss_t cbAnalogPinReport = NULL;
cbf_void_t cbMicrostampReport = NULL;
cbf_void_t cbTimestampReport = NULL;
cbf_short_t cbCongestionReport = NULL;
cbf_void_t cbVersionReport = NULL;
cbf_void_t cbInterrupt = NULL;
cbf_short_t cbEncodingSwitch = NULL;
cbf_void_t cbEmergencyStop1 = NULL;
cbf_void_t cbEmergencyStop2 = NULL;
cbf_void_t cbEmergencyStop3 = NULL;
cbf_void_t cbEmergencyStop4 = NULL;
cbf_void_t cbSystemPause = NULL;
cbf_void_t cbSystemResume = NULL;
cbf_void_t cbSystemHalt = NULL;
cbf_void_t cbSystemReset = NULL;

// sysex events
cbf_void_t cbSysexDigitalPinData = NULL;
cbf_void_t cbSysexAnalogPinData = NULL;
cbf_void_t cbSysexSchedulerData = NULL;
cbf_void_t cbSysexOneWireData = NULL;
cbf_void_t cbSysexUartData = NULL;
cbf_void_t cbSysexI2CData = NULL;
cbf_void_t cbSysexSPIData = NULL;
cbf_void_t cbSysexStringData = NULL;
cbf_void_t cbSysexDigitalPinReport = NULL;
cbf_void_t cbSysexAnalogPinReport = NULL;
cbf_void_t cbSysexVersionReport = NULL;
cbf_void_t cbSysexFeaturesReport = NULL;
cbf_void_t cbSysexPinCapsReq = NULL;
cbf_void_t cbSysexPinCapsRep = NULL;
cbf_void_t cbSysexPinMapReq = NULL;
cbf_void_t cbSysexPinMapRep = NULL;
cbf_void_t cbSysexPinStateReq = NULL;
cbf_void_t cbSysexPinStateRep = NULL;
cbf_void_t cbSysexDeviceReq = NULL;
cbf_void_t cbSysexDeviceRep = NULL;
cbf_void_t cbSysexRCSwitchIn = NULL;
cbf_void_t cbSysexRCSwitchOut = NULL;

// sysex sub: scheduler events
cbf_ss_t cbSchedCreate = NULL;
cbf_short_t cbSchedDelete = NULL;
cbf_sss_t cbSchedAdd = NULL;
cbf_sl_t cbSchedSchedule = NULL;
cbf_long_t cbSchedDelay = NULL;
cbf_void_t cbSchedQueryList = NULL;
cbf_short_t cbSchedQueryTask = NULL;
cbf_void_t cbSchedReset = NULL;

