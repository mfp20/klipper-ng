
#include <libknp.h>
#include <stdarg.h>		// va_arg
#include <string.h>		// strcpy
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

char fwname[] = "klipper-ng";
char fwver[] = "0000000";

// time and performance handling
uint16_t sstart = 0;
uint16_t mstart = 0;
uint16_t ustart = 0;
uint16_t deltaTime = 0;
uint16_t jitter = 0;

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
	// hal
	halInit();
	// protocol
	encodingSwitch(PROTOCOL_ENCODING_NORMAL);
}

void localLog(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf(buffer,format, args);
	cmdSysexStringData(len,buffer);
	va_end (args);
}

void remoteLog(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf(buffer,format, args);
	cmdSysexStringData(len,buffer);
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
		case STATUS_INFO:
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
			eventSystemPause(event[3]);
			break;
		case STATUS_SYSTEM_RESUME:
			eventSystemResume(event[3]);
			break;
		case STATUS_SYSTEM_RESET:
			eventSystemReset(event[3]);
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
					eventSysexOneWire();
					break;
				case SYSEX_UART_DATA:
					eventSysexUart();
					break;
				case SYSEX_I2C_DATA:
					eventSysexI2C();
					break;
				case SYSEX_SPI_DATA:
					eventSysexSPI();
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
				case SYSEX_VERSION:
					switch(event[4]) {
						case SYSEX_SUB_VERSION_FIRMWARE_NAME:
							eventSysexVersion(event[4],fwname);
							break;
						case SYSEX_SUB_VERSION_FIRMWARE_VER:
							eventSysexVersion(event[4],fwver);
							break;
						case SYSEX_SUB_VERSION_LIBKNP:
							sprintf(ver,"%x",RELEASE_LIBKNP);
							eventSysexVersion(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_PROTOCOL:
							sprintf(ver,"%x",RELEASE_PROTOCOL);
							eventSysexVersion(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_HAL:
							sprintf(ver,"%x",RELEASE_HAL);
							eventSysexVersion(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_BOARD:
							sprintf(ver,"%x",RELEASE_BOARD);
							eventSysexVersion(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_ARCH:
							sprintf(ver,"%x",RELEASE_ARCH);
							eventSysexVersion(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_DEFINES:
							sprintf(ver,"%x",RELEASE_DEFINES);
							eventSysexVersion(event[4],ver);
							break;
						case SYSEX_SUB_VERSION_ALL:
							eventSysexVersion(event[4],fwname);
							eventSysexVersion(event[4],fwver);
							sprintf(ver,"%x",RELEASE_LIBKNP);
							eventSysexVersion(event[4],ver);
							sprintf(ver,"%x",RELEASE_PROTOCOL);
							eventSysexVersion(event[4],ver);
							sprintf(ver,"%x",RELEASE_HAL);
							eventSysexVersion(event[4],ver);
							sprintf(ver,"%x",RELEASE_BOARD);
							eventSysexVersion(event[4],ver);
							sprintf(ver,"%x",RELEASE_ARCH);
							eventSysexVersion(event[4],ver);
							sprintf(ver,"%x",RELEASE_DEFINES);
							eventSysexVersion(event[4],ver);
							break;
						default:
							sprintf(ver,"%x",0);
							eventSysexVersion(event[4],ver);
							break;
					}
					break;
				case SYSEX_FEATURES:
					eventSysexFeatures(event[5]);
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
	switch (event[2]) {
		case STATUS_PIN_MODE:
			sprintf(output, "STATUS_PIN_MODE %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_DIGITAL_PORT_REPORT:
			sprintf(output, "STATUS_DIGITAL_PORT_REPORT %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_DIGITAL_PORT_SET:
			sprintf(output, "STATUS_DIGITAL_PORT_SET %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_DIGITAL_PIN_REPORT:
			sprintf(output, "STATUS_DIGITAL_PIN_REPORT %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_DIGITAL_PIN_SET:
			sprintf(output, "STATUS_DIGITAL_PIN_SET %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_ANALOG_PIN_REPORT:
			sprintf(output, "STATUS_ANALOG_PIN_REPORT %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_ANALOG_PIN_SET:
			sprintf(output, "STATUS_ANALOG_PIN_SET %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_PROTOCOL_VERSION:
			sprintf(output, "STATUS_PROTOCOL_VERSION %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_PROTOCOL_ENCODING:
			sprintf(output, "STATUS_PROTOCOL_ENCODING %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_INFO:
			sprintf(output, "STATUS_INFO %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_SIGNAL:
			sprintf(output, "STATUS_SIGNAL %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_INTERRUPT:
			sprintf(output, "STATUS_INTERRUPT %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_EMERGENCY_STOP:
			sprintf(output, "STATUS_EMERGENCY_STOP %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_SYSTEM_PAUSE:
			sprintf(output, "STATUS_SYSTEM_PAUSE %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_SYSTEM_RESUME:
			sprintf(output, "STATUS_SYSTEM_RESUME %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_SYSTEM_RESET:
			sprintf(output, "STATUS_SYSTEM_RESET %d %d %d\n", event[2], event[3], event[4]);
			break;
		case STATUS_SYSEX_START:
			sprintf(output, "STATUS_SYSEX_START (not implemented)\n");
			break;
		default:
			sprintf(output, "Unknown event %#x\n", event[2]);
			break;
	}
}

void newEvent(void) {
	while(binConsole->available(binConsole)) {
		binConsole->read(binConsole, &binData, 1, 1);
		if (decodeEvent(&binData, 0, NULL)) {
			runEvent(0, NULL);
			break;
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
	if(sstart!=seconds()) {
		printf("%4d:%4d:%4d - delta %4d (+%4d)\n",sstart,mstart,ustart,deltaTime,jitter);
		sstart = seconds();
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
		usleep(1); // in case cpu is too fast, mutex in micros/millis can't recover in time
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
	encodedSize = encodeReportDigitalPort(encodedEvent, port, timeout);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSetDigitalPort(uint8_t port, uint8_t value) {
	encodedSize = encodeSetDigitalPort(encodedEvent, port, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdGetDigitalPin(uint8_t pin, uint8_t timeout) {
	encodedSize = encodeReportDigitalPin(encodedEvent, pin, timeout);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSetDigitalPin(uint8_t pin, uint16_t value) {
	encodedSize = encodeSetDigitalPin(encodedEvent, pin, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdGetAnalogPin(uint8_t pin, uint8_t timeout) {
	encodedSize = encodeReportAnalogPin(encodedEvent, pin, timeout);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSetAnalogPin(uint8_t pin, uint16_t value) {
	encodedSize = encodeSetAnalogPin(encodedEvent, pin, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdHandshakeProtocolVersion(void) {
	encodedSize = encodeProtocolVersion(encodedEvent);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdHandshakeEncoding(uint8_t proto) {
	encodedSize = encodeProtocolEncoding(encodedEvent, proto);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdGetInfo(uint8_t info) {
	encodedSize = encodeInfo(encodedEvent, info, 0);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSendSignal(uint8_t sig, uint8_t value) {
	encodedSize = encodeSignal(encodedEvent, sig, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSendInterrupt(uint8_t irq, uint8_t value) {
	encodedSize = encodeInterrupt(encodedEvent, irq, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdEmergencyStop(uint8_t group) {
	encodedSize = encodeEmergencyStop(encodedEvent, group);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSystemPause(uint16_t delay) {
	encodedSize = encodeSystemPause(encodedEvent, delay);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSystemResume(uint16_t time) {
	encodedSize = encodeSystemResume(encodedEvent, time);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSystemReset(uint8_t mode) {
	encodedSize = encodeSystemReset(encodedEvent, mode);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSysexPrefPins(uint8_t cmd, uint8_t pin) {
	encodedSize = encodeSysexPrefPins(encodedEvent, cmd, pin);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSysexPinGroups(uint8_t group, uint8_t cmd, uint8_t pin) {
	encodedSize = encodeSysexPinGroups(encodedEvent, group, cmd, pin);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void cmdSysexDigitalPin(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexAnalogPin(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexOneWireData(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexUartData(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexI2CData(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSPIData(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexStringData(uint8_t len, char *string) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexScheduler(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSchedCreate(uint8_t id, uint8_t len) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSchedDelete(uint8_t id) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSchedAdd(uint8_t id, uint8_t size, uint8_t *event) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSchedSchedule(uint8_t id, uint16_t delay) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSchedDelay(uint16_t delay) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSchedQueryList(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSchedQueryTask(uint8_t id) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexSchedReset(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexVersion(uint8_t item) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexFeatures(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexPinCapsReq(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexPinCapsRep(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexPinMapReq(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexPinMapRep(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexPinStateReq(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexPinStateRep(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexDeviceReq(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexDeviceRep(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexRCSwitchIn(void) {
	stderrPrint("Not Implemented: \n");
}

void cmdSysexRCSwitchOut(void) {
	stderrPrint("Not Implemented: \n");
}


//
// events produced by commands received from the other side of the connection
//

void eventPinMode(uint8_t pin, uint8_t mode) {
	pin_mode(pin, mode);
}

void eventReportDigitalPort(uint8_t port, uint8_t timeout) {
	encodedSize = encodeReportDigitalPort(encodedEvent, port, port_read(port, timeout));
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSetDigitalPort(uint8_t port, uint8_t value) {
	port_write(port, value);
}

void eventReportDigitalPin(uint8_t pin, uint8_t timeout) {
	encodedSize = encodeReportDigitalPin(encodedEvent, pin, pin_read(pin, timeout));
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSetDigitalPin(uint8_t pin, uint16_t value) {
	pin_write(pin, value);
}

void eventReportAnalogPin(uint8_t pin, uint8_t timeout) {
	encodedSize = encodeReportAnalogPin(encodedEvent, pin, pin_read_adc(pin, timeout));
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSetAnalogPin(uint8_t pin, uint16_t value) {
	pin_write_pwm(pin, value);
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
	encodedSize = encodeInfo(encodedEvent, info, value);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventHandleSignal(uint8_t sig, uint8_t key, uint8_t value) {
	switch (sig) {
		case SIG_JITTER:
			// TODO, delay all tasks OR bring clock back
			break;
		case SIG_DISCARD:
			// TODO, re-send event?
			break;
		default:
			break;
	}
}

void eventHandleInterrupt(uint8_t irq, uint8_t key, uint8_t value) {
	switch (irq) {
		case IRQ_PRIORITY:
			// TODO, exec now
			break;
		default:
			break;
	}
}

void eventEmergencyStop(uint8_t g) {
	for(int i;i<16;i++) {
		if (pin_group[g][i]) pin_write(1,0); // TODO
	}
}

void eventSystemPause(uint16_t delay) {
	encodedSize = encodeSystemPause(encodedEvent, halPause(delay));
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSystemResume(uint16_t delay) {
	encodedSize = encodeSystemPause(encodedEvent, halResume(delay));
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSystemReset(uint8_t mode) {
	encodedSize = encodeSystemReset(encodedEvent, mode);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
	halReset(mode);
}

void eventSysexPrefPins(void) {
	remoteLog("Not Implemented: \n");
}

void eventSysexPinGroups(void) {
	remoteLog("Not Implemented: \n");
}

void eventSysexDigitalPin(void) {
	remoteLog("Not Implemented: \n");
}

void eventSysexAnalogPin(void) {
	remoteLog("Not Implemented: \n");
}

void eventSysexOneWire(void) {
	remoteLog("Not Implemented: \n");
}

void eventSysexUart(void) {
	remoteLog("Not Implemented: \n");
}

void eventSysexI2C(void) {
	remoteLog("Not Implemented: \n");
}

void eventSysexSPI(void) {
	remoteLog("Not Implemented: \n");
}

void eventSysexString(uint8_t argc, char *argv) {
	remoteLog("Not Implemented: \n");
}

void eventSysexScheduler(void) {
	remoteLog("Not Implemented: \n");
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
	encodedSize = encodeSysexTask(encodedEvent, task, error);
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

void eventSysexVersion(uint8_t item, char *ver) {
	encodedSize = encodeSysex(encodedEvent, strlen(ver), (uint8_t *)ver);
	binConsole->write(binConsole, encodedEvent, encodedSize, 0);
}

void eventSysexFeatures(uint8_t feature) {
	uint8_t *data;
	switch (feature) {
		case SYSEX_SUB_FEATURES_PIN_TOTAL:
			break;
			//case SYSEX_SUB_FEATURES_PIN_ANALOG_FIRST:
			//	break;
			//case SYSEX_SUB_FEATURES_PIN_ANALOG_TOTAL:
			//	break;
		case SYSEX_SUB_FEATURES_TOTAL:
			break;
		default:
			break;
	}
	encodedSize = encodeSysexFeatures(encodedEvent, feature, data);
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

