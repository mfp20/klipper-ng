
#include <libknp.h>

// time and performance handling
uint16_t mstart = 0;
uint16_t ustart = 0;
uint16_t jitter = 0;
uint8_t txtData, binData, peerData;
uint8_t encodedEvent[PROTOCOL_MAX_EVENT_BYTES];

// tasks handling
task_t *tasks = NULL;
task_t *running = NULL;

void init(void) {
	// init hardware
	halInit();

	// setup protocol callbacks
	printString = logWrite;
	printErr = errWrite;
	encodingSwitch(PROTOCOL_ENCODING_NORMAL);
}

void runEvent(uint8_t size, uint8_t *event) {
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

void runScheduler(uint16_t now) {
	if (tasks) {
		task_t *current = tasks;
		task_t *previous = NULL;
		while (current) {
			if (current->ms > 0 && current->ms < now) { // TODO handle overflow
				uint16_t start = current->ms;
				uint8_t pos = current->pos;
				uint8_t len = current->len;
				uint8_t *messages = current->messages;
				uint8_t reschedule = 0;
				running = current;
				while (pos < len) {
					if (decodeEvent(&messages[pos++], 0, NULL))
						runEvent(eventSize, eventBuffer);
					if (start != current->ms) { // return true if task got rescheduled during run.
						current->pos = ( pos == len ? 0 : pos ); // last message executed? -> start over next time
						reschedule = 1;
						break;
					}
				}
				running = NULL;
				if (reschedule) {
					previous = current;
					current = current->next;
				} else {
					if (previous) {
						previous->next = current->next;
						free(current);
						current = previous->next;
					} else {
						tasks = current->next;
						free(current);
						current = tasks;
					}
				}
			} else {
				current = current->next;
			}
		}
	}
}

void run(void) {
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
	deltaTime = uelapsed(mstart, ustart, micros(), millis());
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
		deltaTime = uelapsed(mstart, ustart, micros(), millis());
	}
	// evaluate jitter
	jitter = jitter+(deltaTime-TICKRATE);
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
	binConsole->write(binConsole, encodedEvent, evsize, 0);
}

void eventVersionReport(void) {
	uint8_t byte = PROTOCOL_VERSION_MINOR;
	uint8_t evsize = encodeEvent(STATUS_VERSION_REPORT, PROTOCOL_VERSION_MAJOR, &byte, encodedEvent);
	binConsole->write(binConsole, encodedEvent, evsize, 0);
}

void eventInterrupt(void) {
	errWrite("Not Implemented: \n");
}

void eventEncodingSwitch(uint8_t proto) {
	if (proto==PROTOCOL_ENCODING_REPORT) {
		proto = encodingSwitch(proto);
		uint8_t evsize = encodeEvent(STATUS_ENCODING_SWITCH, proto, NULL, encodedEvent);
		binConsole->write(binConsole, encodedEvent, evsize, 0);
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
	binConsole->write(binConsole, encodedEvent, evsize, 0);
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
// host commands to trigger events on mcu
//

void cmdPinMode(uint8_t pin, uint8_t mode) {
	errWrite("Not Implemented: \n");
}

void cmdDigitalPortData(uint8_t port, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void cmdDigitalPinData(uint8_t pin, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void cmdAnalogPinData(uint8_t pin, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void cmdDigitalPortReport(uint8_t port, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void cmdDigitalPinReport(uint8_t port, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void cmdAnalogPinReport(uint8_t pin, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void cmdMicrostampReport(void) {
	errWrite("Not Implemented: \n");
}

void cmdTimestampReport(void) {
	errWrite("Not Implemented: \n");
}

void cmdCongestionReport(uint8_t lag) {
	errWrite("Not Implemented: \n");
}

void cmdVersionReport(void) {
	errWrite("Not Implemented: \n");
}

void cmdInterrupt(void) {
	errWrite("Not Implemented: \n");
}

void cmdEncodingSwitch(uint8_t proto) {
	errWrite("Not Implemented: \n");
}

void cmdEmergencyStop1(void) {
	errWrite("Not Implemented: \n");
}

void cmdEmergencyStop2(void) {
	errWrite("Not Implemented: \n");
}

void cmdEmergencyStop3(void) {
	errWrite("Not Implemented: \n");
}

void cmdEmergencyStop4(void) {
	errWrite("Not Implemented: \n");
}

void cmdSystemPause(void) {
	errWrite("Not Implemented: \n");
}

void cmdSystemResume(void) {
	errWrite("Not Implemented: \n");
}

void cmdSystemHalt(void) {
	errWrite("Not Implemented: \n");
}

void cmdSystemReset(void) {
	errWrite("Not Implemented: \n");
}


void cmdSysexDigitalPinData(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexAnalogPinData(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedulerData(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexOneWireData(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexUartData(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexI2CData(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSPIData(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexStringData(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexDigitalPinReport(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexAnalogPinReport(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexVersionReport(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexFeaturesReport(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexPinCapsReq(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexPinCapsRep(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexPinMapReq(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexPinMapRep(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexPinStateReq(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexPinStateRep(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexDeviceReq(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexDeviceRep(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexRCSwitchIn(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexRCSwitchOut(void) {
	errWrite("Not Implemented: \n");
}

void cmdSchedCreate(uint8_t id, uint8_t len) {
	errWrite("Not Implemented: \n");
}

void cmdSchedDelete(uint8_t id) {
	errWrite("Not Implemented: \n");
}

void cmdSchedAdd(uint8_t id, uint8_t additionalBytes, uint8_t *message) {
	errWrite("Not Implemented: \n");
}

void cmdSchedSchedule(uint8_t id, uint16_t delay) {
	errWrite("Not Implemented: \n");
}

void cmdSchedDelay(uint16_t delay) {
	errWrite("Not Implemented: \n");
}

void cmdSchedQueryList(void) {
	errWrite("Not Implemented: \n");
}

void cmdSchedQueryTask(uint8_t id) {
	errWrite("Not Implemented: \n");
}

void cmdSchedReset(void) {
	errWrite("Not Implemented: \n");
}

