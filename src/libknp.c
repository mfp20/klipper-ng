
#include <libknp.h>
#include <stdarg.h>		// va_arg
#include <string.h>		// strcpy

char fwname[] = "klipper-ng";
char fwver[] = "0000000";

// time and performance handling
uint16_t mstart = 0;
uint16_t ustart = 0;
uint16_t jitter = 0;
uint16_t deltaTime = 0;

// data
uint8_t txtData, binData, peerData;
uint8_t encodedEvent[PROTOCOL_MAX_EVENT_BYTES];
uint8_t encodedSize = 0;

// tasks handling
task_t *tasks = NULL;
task_t *running = NULL;

void init(char *name, char *ver) {
	if (name) strcpy(fwname, name);
	if (ver) strcpy(fwver, ver);

	// init hardware
	halInit();

	// setup protocol callbacks
	encodingSwitch(PROTOCOL_ENCODING_NORMAL);
}

void remoteLog(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf(buffer,format, args);
	eventSysexStringData(len,buffer);
	va_end (args);
}

void runEvent(uint8_t size, uint8_t *event) {
	uint8_t datalen = 0;
	char ver[7];
	// TODO fix all arguments
	switch(event[0]) {
		case STATUS_PIN_MODE:
			eventPinMode(event[1], event[2]);
			break;
		case STATUS_DIGITAL_PORT_REPORT:
			eventReportDigitalPort(event[3], event[3]);
			break;
		case STATUS_DIGITAL_PORT_SET:
			eventSetDigitalPort(event[3], event[3]);
			break;
		case STATUS_DIGITAL_PIN_REPORT:
			eventReportDigitalPin(event[3], event[3]);
			break;
		case STATUS_DIGITAL_PIN_SET:
			eventSetDigitalPin(event[3], event[3]);
			break;
		case STATUS_ANALOG_PIN_REPORT:
			eventReportAnalogPin(event[3], event[3]);
			break;
		case STATUS_ANALOG_PIN_SET:
			eventSetAnalogPin(event[3], event[3]);
			break;
		case STATUS_PROTOCOL_VERSION:
			eventHandshakeProtocolVersion();
			break;
		case STATUS_PROTOCOL_ENCODING:
			eventHandshakeEncoding(event[32]);
			break;
		case STATUS_INFO_REQ:
			eventReportInfo(event[1], event[2]);
			break;
		case STATUS_SIGNAL:
			eventHandleSignal(event[12], event[12], event[12]);
			break;
		case STATUS_INTERRUPT:
			eventHandleInterrupt(event[12], event[12], event[12]);
			break;
		case STATUS_EMERGENCY_STOP:
			eventEmergencyStop(event[4]);
			break;
		case STATUS_SYSTEM_PAUSE:
			eventSystemPause(event[4]);
			break;
		case STATUS_SYSTEM_RESUME:
			eventSystemResume(event[4]);
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
							remoteLog("Not Implemented: %d (SYSEX_EXTEND)\n", event[1]);
					}
					break;
				case SYSEX_DIGITAL_PIN_DATA:
					eventSysexDigitalPin();
					break;
				case SYSEX_ANALOG_PIN_DATA:
					eventSysexAnalogPin();
					break;
				case SYSEX_SCHEDULER_DATA:
					if (size > 0) {
						switch (event[0]) {
							case SYSEX_SUB_SCHED_CREATE:
								if (size == 4) {
									eventSysexSchedCreate(event[1], event[2] | event[3] << 7);
								}
								break;
							case SYSEX_SUB_SCHED_ADD:
								if (size > 2) {
									eventSysexSchedAdd(event[1], size - 2, event + 2); // addToTask copies data...
								}
								break;
							case SYSEX_SUB_SCHED_DELAY:
								if (size == 6) {
									//event++; compiler error after moving the sysex switch
									eventSysexSchedDelay(*(long*)((uint8_t*)event));
								}
								break;
							case SYSEX_SUB_SCHED_SCHEDULE:
								if (size == 7) { // one byte taskid, 5 bytes to encode 4 bytes of long
									eventSysexSchedSchedule(event[1], *(long*)((uint8_t*)event + 2)); // event[1] | event[2]<<8 | event[3]<<16 | event[4]<<24
								}
								break;
							case SYSEX_SUB_SCHED_LIST_REQ:
								eventSysexSchedQueryList();
								break;
							case SYSEX_SUB_SCHED_TASK_REQ:
								if (size == 2) {
									eventSysexSchedQueryTask(event[1]);
								}
								break;
							case SYSEX_SUB_SCHED_DELETE:
								if (size == 2) {
									eventSysexSchedDelete(event[1]);
								}
								break;
							case SYSEX_SUB_SCHED_RESET:
								eventSysexSchedReset();
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
					if (txtConsole) {
						// The string length will only be at most half the size of the
						// stored input buffer so we can decode the string within the buffer.
						datalen = (size-1)/2;
						cbDecoder(event, datalen, event);
						// Make sure string is null terminated.
						if (event[datalen-1]!='\0') event[datalen] = '\0';
						txtSend(datalen, event);
					}
					break;
				case SYSEX_DIGITAL_PIN_REPORT:
					eventSysexDigitalPinReport();
					break;
				case SYSEX_ANALOG_PIN_REPORT:
					eventSysexAnalogPinReport();
					break;
				case SYSEX_VERSION_REPORT:
					switch(event[4]) {
						case SYSEX_SUB_VERSION_FIRMWARE_NAME:
							eventSysexVersionReport(event[4],fwname);
							break;
						case SYSEX_SUB_VERSION_FIRMWARE_VER:
							eventSysexVersionReport(event[4],fwver);
							break;
						case SYSEX_SUB_VERSION_LIBKNP:
							sprintf(ver,"%x",RELEASE_LIBKNP);
							eventSysexVersionReport(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_PROTOCOL:
							sprintf(ver,"%x",RELEASE_PROTOCOL);
							eventSysexVersionReport(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_HAL:
							sprintf(ver,"%x",RELEASE_HAL);
							eventSysexVersionReport(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_BOARD:
							sprintf(ver,"%x",RELEASE_BOARD);
							eventSysexVersionReport(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_ARCH:
							sprintf(ver,"%x",RELEASE_ARCH);
							eventSysexVersionReport(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_DEFINES:
							sprintf(ver,"%x",RELEASE_DEFINES);
							eventSysexVersionReport(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_ALL:
							eventSysexVersionReport(event[4],fwname);
							eventSysexVersionReport(event[4],fwver);
							sprintf(ver,"%x",RELEASE_LIBKNP);
							eventSysexVersionReport(event[4],ver);
							sprintf(ver,"%x",RELEASE_PROTOCOL);
							eventSysexVersionReport(event[4],ver);
							sprintf(ver,"%x",RELEASE_HAL);
							eventSysexVersionReport(event[4],ver);
							sprintf(ver,"%x",RELEASE_BOARD);
							eventSysexVersionReport(event[4],ver);
							sprintf(ver,"%x",RELEASE_ARCH);
							eventSysexVersionReport(event[4],ver);
							sprintf(ver,"%x",RELEASE_DEFINES);
							eventSysexVersionReport(event[4],ver);
							break;
						default:
							sprintf(ver,"%x",0);
							eventSysexVersionReport(event[4],ver);
							break;
					}
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
					remoteLog("Not Implemented: %d (Unknown command)\n", event[1]);
					break;
			}
			break;
		default:
			break;
	}
}

void printEvent(uint8_t size, uint8_t *event, char *output) {
	if (event == NULL) {
		size = eventSize;
		event = eventBuffer;
	}
	switch (event[3]) {
		case STATUS_PIN_MODE:
		case STATUS_DIGITAL_PORT_REPORT:
		case STATUS_DIGITAL_PORT_SET:
		case STATUS_DIGITAL_PIN_REPORT:
		case STATUS_DIGITAL_PIN_SET:
		case STATUS_ANALOG_PIN_REPORT:
		case STATUS_ANALOG_PIN_SET:
		case STATUS_PROTOCOL_VERSION:
		case STATUS_PROTOCOL_ENCODING:
		case STATUS_INFO_REQ:
		case STATUS_INFO_REP:
		case STATUS_SIGNAL:
		case STATUS_INTERRUPT:
		case STATUS_EMERGENCY_STOP:
		case STATUS_SYSTEM_PAUSE:
		case STATUS_SYSTEM_RESUME:
		case STATUS_SYSTEM_HALT:
		case STATUS_SYSTEM_RESET:
		case STATUS_SYSEX_START:
			if (output) {
			} else {
				txtWrite("Not implemented");
			}
			break;
		default:
			txtWrite("Unknown event %#x", event[3]);
			break;
	}
}

void newEvent(void) {
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
}

void schedEvent(uint16_t now) {
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
	if (jitter>=TICKTIME) {
		uint16_t jms = jitter/1000;
		cmdSendSignal(SIG_JITTER, jms>126?127:jms);
		jitter = jitter%1000;
	}

	// 1. run hardware tasks
	halRun();

	// 2. get new event
	newEvent();

	// 3. run scheduled events
	schedEvent(mstart);

	// --- evaluate spare time
	deltaTime = uelapsed(mstart, ustart, micros(), millis());
	if (deltaTime >= TICKTIME) {
		jitter = jitter+(deltaTime-TICKTIME);
		return;
	}

	// --- spend spare time waiting for new events
	while(deltaTime<TICKTIME) {
		// wait for new incoming event
		newEvent();
		// evaluate elapsed time
		deltaTime = uelapsed(mstart, ustart, micros(), millis());
	}
	// evaluate jitter
	jitter = jitter+(deltaTime-TICKTIME);
}

//
// commands to trigger events on the other side of the connection
//

void cmdPinMode(uint8_t pin, uint8_t mode) {
	encodedSize = encodePinMode(encodedEvent, pin, mode);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdGetDigitalPort(uint8_t port, uint8_t timeout) {
	encodedSize = encodeReportDigitalPort(encodedEvent, port);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSetDigitalPort(uint8_t port, uint8_t value) {
	encodedSize = encodeSetDigitalPort(encodedEvent, port, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdGetDigitalPin(uint8_t pin, uint8_t timeout) {
	encodedSize = encodeReportDigitalPin(encodedEvent, pin);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSetDigitalPin(uint8_t pin, uint8_t value) {
	encodedSize = encodeSetDigitalPin(encodedEvent, pin, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdGetAnalogPin(uint8_t pin, uint8_t timeout) {
	encodedSize = encodeReportAnalogPin(encodedEvent, pin);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSetAnalogPin(uint8_t pin, uint8_t value) {
	encodedSize = encodeSetAnalogPin(encodedEvent, pin, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdHandshakeProtocolVersion(void) {
	encodedSize = encodeProtocolVersion(encodedEvent);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdHandshakeEncoding(uint8_t proto) {
	errWrite("Not Implemented: \n");
}

void cmdGetInfo(uint8_t info) {
	errWrite("Not Implemented: \n");
}

void cmdSendSignal(uint8_t sig, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void cmdSendInterrupt(uint8_t irq, uint8_t value) {
	errWrite("Not Implemented: \n");
}

void cmdEmergencyStop(uint8_t group) {
	errWrite("Not Implemented: \n");
}

void cmdSystemPause(uint16_t delay) {
	errWrite("Not Implemented: \n");
}

void cmdSystemResume(uint16_t time) {
	errWrite("Not Implemented: \n");
}

void cmdSystemHalt(void) {
	errWrite("Not Implemented: \n");
}

void cmdSystemReset(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexDigitalPin(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexAnalogPin(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexScheduler(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedCreate(uint8_t id, uint8_t len) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedDelete(uint8_t id) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedAdd(uint8_t id, uint8_t size, uint8_t *event) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedSchedule(uint8_t id, uint16_t delay) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedDelay(uint16_t delay) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedQueryList(void) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedQueryTask(uint8_t id) {
	errWrite("Not Implemented: \n");
}

void cmdSysexSchedReset(void) {
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

void cmdSysexVersionReport(uint8_t item) {
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


//
// events produced by commands received from the other side of the connection
//

void eventPinMode(uint8_t pin, uint8_t mode) {
	remoteLog("Not Implemented: \n");
}

void eventReportDigitalPort(uint8_t port, uint8_t timeout) {
	// TODO
	uint8_t value = 0;
	encodedSize = encodeReportDigitalPort(encodedEvent, port);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSetDigitalPort(uint8_t port, uint8_t value) {
	remoteLog("Not Implemented: \n");
}

void eventReportDigitalPin(uint8_t pin, uint8_t timeout) {
	// TODO
	uint8_t value = 0;
	encodedSize = encodeReportDigitalPin(encodedEvent, pin);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSetDigitalPin(uint8_t pin, uint8_t value) {
	remoteLog("Not Implemented: \n");
}

void eventReportAnalogPin(uint8_t pin, uint8_t timeout) {
	// TODO
	uint8_t value = 0;
	encodedSize = encodeReportAnalogPin(encodedEvent, pin);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSetAnalogPin(uint8_t pin, uint8_t value) {
	remoteLog("Not Implemented: \n");
}

void eventHandshakeProtocolVersion(void) {
	encodedSize = encodeProtocolVersion(encodedEvent);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventHandshakeEncoding(uint8_t proto) {
	if (proto==PROTOCOL_ENCODING_REPORT) {
		encodedSize = encodeProtocolEncoding(encodedEvent, encodingSwitch(proto));
		binConsole->write(binConsole, encodedEvent, encodedSize, 0);
	} else {
		encodingSwitch(proto);
	}
}

void eventReportInfo(uint8_t info, uint8_t value) {
	encodedSize = encodeInfoRep(encodedEvent, info, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventHandleSignal(uint8_t sig, uint8_t key, uint8_t value) {
	// TODO
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventHandleInterrupt(uint8_t irq, uint8_t key, uint8_t value) {
	remoteLog("Not Implemented: \n");
}

void eventEmergencyStop(uint8_t group) {
	remoteLog("Not Implemented: \n");
}

void eventSystemPause(uint16_t delay) {
	remoteLog("Not Implemented: \n");
}

void eventSystemResume(uint16_t delay) {
	remoteLog("Not Implemented: \n");
}

void eventSystemHalt(void) {
	remoteLog("Not Implemented: \n");
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

void eventSysexDigitalPin(void) {
	remoteLog("Not Implemented: \n");
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexAnalogPin(void) {
	remoteLog("Not Implemented: \n");
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexScheduler(void) {
	remoteLog("Not Implemented: \n");
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
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
	encodedSize = encodeTask(encodedEvent, task, error);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexSchedCreate(uint8_t id, uint8_t len) {
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

void eventSysexSchedDelete(uint8_t id) {
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

void eventSysexSchedAdd(uint8_t id, uint8_t additionalBytes, uint8_t *message) {
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

void eventSysexSchedSchedule(uint8_t id, uint16_t delay) {
	task_t *existing = findTask(id);
	if (existing) {
		existing->pos = 0;
		existing->ms = millis() + delay;
	} else {
		reportTask(NULL, true);
	}
}

void eventSysexSchedDelay(uint16_t delay_ms) {
	if (running) {
		long now = millis();
		running->ms += delay_ms;
		if (running->ms < now) { // if delay time allready passed by schedule to 'now'.
			running->ms = now;
		}
	}
}

void eventSysexSchedQueryList(void) {
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

void eventSysexSchedQueryTask(uint8_t id) {
	task_t *task = findTask(id);
	reportTask(task, false);
}

void eventSysexSchedReset(void) {
	while (tasks) {
		task_t *next = tasks->next;
		free(tasks);
		tasks = next;
	}
}

void eventSysexOneWireData(void) {
	remoteLog("Not Implemented: \n");
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexUartData(void) {
	remoteLog("Not Implemented: \n");
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexI2CData(void) {
	remoteLog("Not Implemented: \n");
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexSPIData(void) {
	remoteLog("Not Implemented: \n");
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexStringData(uint8_t argc, char *argv) {
	// TODO check casting from char* to uint8_t*
	encodedSize = encodeSysex(encodedEvent, argc, (uint8_t*)argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexDigitalPinReport(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexAnalogPinReport(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexVersionReport(uint8_t item, char *ver) {
	encodedSize = encodeSysex(encodedEvent, strlen(ver), (uint8_t *)ver);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexFeaturesReport(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexPinCapsReq(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexPinCapsRep(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexPinMapReq(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexPinMapRep(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexPinStateReq(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexPinStateRep(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexDeviceReq(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexDeviceRep(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexRCSwitchIn(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexRCSwitchOut(void) {
	// TODO encodedSize = encodeSysex(encodedEvent, argc, argv);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

