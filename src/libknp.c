
#include <libknp.h>

uint16_t mstart = 0;
uint16_t ustart = 0;
uint16_t uelapsed = 1000;
uint16_t jitter = 0;
uint8_t txtData, binData, peerData;

task_t *tasks = NULL;
task_t *running = NULL;

// SIMPLE SYSTEM

void cmdSystemReset(void) {
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

void cmdReportProtocolVersion(void) {
	uint8_t byte = PROTOCOL_VERSION_MINOR;
	writeCmd(CMD_VERSION_REPORT, PROTOCOL_VERSION_MAJOR, &byte);
}

// SIMPLE PINs

void cmdPinMode(uint8_t pin, uint8_t mode) {
	errWrite("Not Implemented: cmdPinMode\n");
}

void cmdPinAnalog(uint8_t pin, uint8_t value) {
	errWrite("Not Implemented: cmdPinAnalog\n");
}

void cmdPinDigital(uint8_t port, uint8_t value) {
	errWrite("Not Implemented: cmdPinDigital\n");
}

void cmdPinDigitalSetValue(uint8_t pin, uint8_t value) {
	errWrite("Not Implemented: cmdPinDigitalSetValue\n");
}

void cmdReportPinAnalog(uint8_t pin, uint8_t en) {
	errWrite("Not Implemented: cmdReportPinAnalog\n");
}

void cmdReportPinDigital(uint8_t port, uint8_t en) {
	errWrite("Not Implemented: cmdReportPinDigital\n");
}

// SYSEX SYSTEM

void cmdReportFirmwareVersion(void) {
	errWrite("Not Implemented: cmdReportFirmwareVersion\n");
}

void cmdRealtimeServer(uint16_t timeout) {
	bool loop = true;
	uint8_t query = 0;
	uint8_t answer = SYSEX_REALTIME_END;
	uint8_t data = 0;
	uint16_t mstart, ustart, uend, mend;
	if (timeout > 999) timeout = 999;
	mstart = millis();
	while(loop) {
		ustart = micros();
		binConsole->read(binConsole, &query, 1, timeout);
		switch (query) {
			case SYSEX_SUB_REALTIME_SYN:
				answer = SYSEX_SUB_REALTIME_ACK;
				binConsole->write(binConsole, &answer, 1, 0);
				break;
			case SYSEX_SUB_REALTIME_ACK:
				answer = micros(); // TODO get micros MSB only;
				binConsole->write(binConsole, &answer, 1, 0);
				binConsole->read(binConsole, &data, 1, timeout);
				break;
			case SYSEX_SUB_REALTIME_NACK:
				answer = 1; // TODO error code
				binConsole->write(binConsole, &answer, 1, 0);
				break;
			case SYSEX_SUB_REALTIME_TIME_RESET: // sync microseconds
				//hal_time_reset();
				answer = micros(); // TODO get micros MSB only;
				binConsole->write(binConsole, &answer, 1, 1);
				binConsole->read(binConsole, &data, 1, timeout); // receive offset
				//hal_time_adj(data);
				break;
			case SYSEX_SUB_REALTIME_FIN:
				answer = SYSEX_REALTIME_END;
				binConsole->write(binConsole, &answer, 1, 0);
				break;
			case SYSEX_REALTIME_END: // exit 'realtime mode'
			default:
				answer = SYSEX_REALTIME_END;
				loop = false;
				binConsole->write(binConsole, &answer, 1, 0);
				break;
		}
		mend = millis();
		uend = micros();
		if (elapsed(mstart, ustart, uend, mend)>=timeout) break;
	}
}

uint8_t realtimeClient(uint8_t rpc) {
	bool loop = true;
	uint8_t query = SYSEX_SUB_REALTIME_ACK;
	uint8_t data = 0;
	uint16_t mstart, ustart, uend, mend;
	uint16_t timeout = timeout;
	if (timeout > 999) timeout = 999;
	mstart = millis();
	ustart = micros();
	switch (rpc) {
		case SYSEX_SUB_REALTIME_ACK: // ping-pong to measure RR time
			// TODO
			break;
		case SYSEX_SUB_REALTIME_TIME_RESET: // sync microseconds
			// TODO
			break;
		case SYSEX_REALTIME_END: // exit 'realtime mode'
		default:
			query = SYSEX_REALTIME_END;
			binConsole->write(binConsole, &query, 1, 0);
			break;
	}
	return data;
}

// SYSEX SUB SCHEDULER

static bool execute(task_t *task) {
	long start = task->ms;
	int pos = task->pos;
	int len = task->len;
	uint8_t *messages = task->messages;
	running = task;
	while (pos < len) {
		if (receiveMsg(messages[pos++]))
			readCmd(0, NULL);
		if (start != task->ms) { // return true if task got rescheduled during run.
			task->pos = ( pos == len ? 0 : pos ); // last message executed? -> start over next time
			running = NULL;
			return true;
		}
	}
	running = NULL;
	return false;
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

static void reportTask(uint8_t id, task_t *task, bool error) {
	uint8_t byte = CMD_SYSEX_START;
	binConsole->write(binConsole, &byte, 1, 0);
	byte = SYSEX_SCHEDULER_DATA;
	binConsole->write(binConsole, &byte, 1, 0);
	if (error) {
		byte = SYSEX_SUB_SCHED_REPLY_ERROR;
		binConsole->write(binConsole, &byte, 1, 0);
	} else {
		byte = SYSEX_SUB_SCHED_REPLY_QUERY_TASK;
		binConsole->write(binConsole, &byte, 1, 0);
	}
	binConsole->write(binConsole, &id, 1, 0);
	if (task) {
		// don't write first 3 bytes (task_t*, byte); makes use of AVR byteorder (LSB first)
		uint8_t len = firmata_task_len(task);
		uint8_t result[len];
		encode7bitCompat(&((uint8_t *)task)[3], len, result);
		binConsole->write(binConsole, result, len, 0);
	}
	byte = CMD_SYSEX_END;
	binConsole->write(binConsole, &byte, 1, 0);
}

void cmdCreateTask(uint8_t id, int len) {
	task_t *existing = findTask(id);
	if (existing) {
		reportTask(id, existing, true);
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

void cmdDeleteTask(uint8_t id) {
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

void cmdAddToTask(uint8_t id, int additionalBytes, uint8_t *message) {
	task_t *existing = findTask(id);
	if (existing) { //task exists and has not been fully loaded yet
		if (existing->pos + additionalBytes <= existing->len) {
			for (int i = 0; i < additionalBytes; i++) {
				existing->messages[existing->pos++] = message[i];
			}
		}
	} else {
		reportTask(id, NULL, true);
	}
}

void cmdScheduleTask(uint8_t id, long delay_ms) {
	task_t *existing = findTask(id);
	if (existing) {
		existing->pos = 0;
		existing->ms = millis() + delay_ms;
	} else {
		reportTask(id, NULL, true);
	}
}

void cmdDelayTask(long delay_ms) {
	if (running) {
		long now = millis();
		running->ms += delay_ms;
		if (running->ms < now) { // if delay time allready passed by schedule to 'now'.
			running->ms = now;
		}
	}
}

void cmdQueryAllTasks(void) {
	uint8_t byte = CMD_SYSEX_START;
	binConsole->write(binConsole, &byte, 1, 0);
	byte = SYSEX_SCHEDULER_DATA;
	binConsole->write(binConsole, &byte, 1, 0);
	byte = SYSEX_SUB_SCHED_REPLY_QUERY_ALL;
	binConsole->write(binConsole, &byte, 1, 0);
	task_t *task = tasks;
	while (task) {
		binConsole->write(binConsole, &task->id, 1, 0);
		task = task->next;
	}
	byte = CMD_SYSEX_END;
	binConsole->write(binConsole, &byte, 1, 0);
}

void cmdQueryTask(uint8_t id) {
	task_t *task = findTask(id);
	reportTask(id, task, false);
}

void cmdSchedulerReset(void) {
	while (tasks) {
		task_t *next = tasks->next;
		free(tasks);
		tasks = next;
	}
}

// ------------------------

void readCmd(uint8_t len, uint8_t *msg) {
	if (msg == NULL) msg = storedInputData;
	uint8_t datalen = 0;
	switch(msg[0]) {
		case CMD_PIN_MODE:
			cmdPinMode(msg[1], msg[2]);
			break;
		case CMD_DIGITAL_PORT_DATA:
			cmdPinDigital(multiByteChannel, (msg[1] << 7) + msg[2]);
			break;
		case CMD_DIGITAL_PIN_DATA:
			cmdPinDigitalSetValue(msg[1], msg[2]);
			break;
		case CMD_ANALOG_PIN_DATA:
			cmdPinAnalog(multiByteChannel, (msg[1] << 7) + msg[2]);
			break;
		case CMD_DIGITAL_PORT_REPORT:
			break;
		case CMD_DIGITAL_PIN_REPORT:
			cmdReportPinDigital(multiByteChannel, msg[1]);
			break;
		case CMD_ANALOG_PIN_REPORT:
			cmdReportPinAnalog(multiByteChannel, msg[1]);
			break;
		case CMD_SYSTEM_RESET:
			cmdSystemReset();
			break;
		case CMD_VERSION_REPORT:
			cmdReportProtocolVersion();
			break;
		case CMD_SYSEX_START:
			switch (msg[1]) { // first byte is sysex start, second byte is command
				case SYSEX_EXTEND:
					switch(msg[2]+msg[3]) { // second byte is SYSEX_EXTEND, third&forth bytes are the extended commands
						default:
							errWrite("Not Implemented: %d (SYSEX_EXTEND)\n", msg[1]);
					}
					break;
				case SYSEX_DIGITAL_PIN_DATA:
					errWrite("Not Implemented: %d (SYSEX_REPORT_DIGITAL_PIN)\n", msg[1]);
					break;
				case SYSEX_ANALOG_PIN_DATA:
					errWrite("Not Implemented: %d (SYSEX_EXTENDED_REPORT_ANALOG)\n", msg[1]);
					break;
				case SYSEX_SCHEDULER_DATA:
					if (sysexBytesRead > 0) {
						switch (storedInputData[0]) {
							case SYSEX_SUB_SCHED_CREATE:
								if (sysexBytesRead == 4) {
									cmdCreateTask(storedInputData[1], storedInputData[2] | storedInputData[3] << 7);
								}
								break;
							case SYSEX_SUB_SCHED_ADD:
								if (sysexBytesRead > 2) {
									int len = num7BitOutbytes(sysexBytesRead - 2);
									decode7bitCompat(storedInputData + 2, len, storedInputData + 2); // decode inplace
									cmdAddToTask(storedInputData[1], len, storedInputData + 2); // addToTask copies data...
								}
								break;
							case SYSEX_SUB_SCHED_DELAY:
								if (sysexBytesRead == 6) {
									//storedInputData++; compiler error after moving the sysex switch
									decode7bitCompat(storedInputData, 4, storedInputData); // decode inplace
									cmdDelayTask(*(long*)((uint8_t*)storedInputData));
								}
								break;
							case SYSEX_SUB_SCHED_SCHEDULE:
								if (sysexBytesRead == 7) { // one byte taskid, 5 bytes to encode 4 bytes of long
									decode7bitCompat(storedInputData + 2, 4, storedInputData + 2); // decode inplace
									cmdScheduleTask(storedInputData[1], *(long*)((uint8_t*)storedInputData + 2)); // storedInputData[1] | storedInputData[2]<<8 | storedInputData[3]<<16 | storedInputData[4]<<24
								}
								break;
							case SYSEX_SUB_SCHED_QUERY_ALL:
								cmdQueryAllTasks();
								break;
							case SYSEX_SUB_SCHED_QUERY_TASK:
								if (sysexBytesRead == 2) {
									cmdQueryTask(storedInputData[1]);
								}
								break;
							case SYSEX_SUB_SCHED_DELETE:
								if (sysexBytesRead == 2) {
									cmdDeleteTask(storedInputData[1]);
								}
								break;
							case SYSEX_SUB_SCHED_RESET:
								cmdSchedulerReset();
						}
					}
					break;
				case SYSEX_ONEWIRE_DATA:
					errWrite("Not Implemented: %d (SYSEX_ONEWIRE_DATA)\n", msg[1]);
					break;
				case SYSEX_UART_DATA:
					errWrite("Not Implemented: %d (SYSEX_SERIAL_DATA2)\n", msg[1]);
					break;
				case SYSEX_I2C_DATA:
					errWrite("Not Implemented: %d (SYSEX_I2C_REQUEST)\n", msg[1]);
					break;
				case SYSEX_SPI_DATA:
					errWrite("Not Implemented: %d (SYSEX_SPI_DATA)\n", msg[1]);
					break;
				case SYSEX_STRING_DATA:
					// The string length will only be at most half the size of the
					// stored input buffer so we can decode the string within the buffer.
					datalen = (sysexBytesRead-1)/2;
					cbDecoder(storedInputData, datalen, storedInputData);
					// Make sure string is null terminated. This may be the case for data
					// coming from client libraries in languages that don't null terminate
					// strings.
					if (storedInputData[datalen-1]!='\0') storedInputData[datalen] = '\0';
					logWrite((char *)storedInputData);
					break;
				case SYSEX_CRC_DATA:
					errWrite("Not Implemented: %d (SYSEX_SPI_DATA)\n", msg[1]);
					break;
				case SYSEX_DIGITAL_PIN_REPORT:
					errWrite("Not Implemented: %d (SYSEX_REPORT_DIGITAL_PIN)\n", msg[1]);
					break;
				case SYSEX_ANALOG_PIN_REPORT:
					errWrite("Not Implemented: %d (SYSEX_EXTENDED_REPORT_ANALOG)\n", msg[1]);
					break;
				case SYSEX_VERSION_REPORT:
					//dispatchSysex(msg[1], 0, NULL);
					cmdReportFirmwareVersion();
					break;
				case SYSEX_FEATURES_REPORT:
					errWrite("Not Implemented: %d (SYSEX_REPORT_FEATURES)\n", msg[1]);
					break;
				case SYSEX_PINCAPS_REQ:
					errWrite("Not Implemented: %d (SYSEX_CAPABILITY_QUERY)\n", msg[1]);
					break;
				case SYSEX_PINCAPS_REP:
					errWrite("Not Implemented: %d (SYSEX_CAPABILITY_RESPONSE)\n", msg[1]);
					break;
				case SYSEX_PINMAP_REQ:
					errWrite("Not Implemented: %d (SYSEX_ANALOG_MAPPING_QUERY)\n", msg[1]);
					break;
				case SYSEX_PINMAP_REP:
					errWrite("Not Implemented: %d (SYSEX_ANALOG_MAPPING_RESPONSE)\n", msg[1]);
					break;
				case SYSEX_PINSTATE_REQ:
					errWrite("Not Implemented: %d (SYSEX_PIN_STATE_QUERY)\n", msg[1]);
					break;
				case SYSEX_PINSTATE_REP:
					errWrite("Not Implemented: %d (SYSEX_PIN_STATE_RESPONSE)\n", msg[1]);
					break;
				case SYSEX_DEVICE_REQ:
					errWrite("Not Implemented: %d (SYSEX_DEVICE_QUERY)\n", msg[1]);
					break;
				case SYSEX_DEVICE_REP:
					errWrite("Not Implemented: %d (SYSEX_DEVICE_RESPONSE)\n", msg[1]);
					break;
				case SYSEX_RCSWITCH_OUT:
					errWrite("Not Implemented: %d (SYSEX_RCOUT)\n", msg[1]);
					break;
				case SYSEX_RCSWITCH_IN:
					errWrite("Not Implemented: %d (SYSEX_RCIN)\n", msg[1]);
					break;
				case SYSEX_REALTIME_END:
					cmdRealtimeServer(TICKRATE);
					break;
				case SYSEX_REALTIME_START:
					cmdRealtimeServer(TICKRATE);
					break;
				default:
					//dispatchSysex(msg[1], sysexBytesRead - 1, msg + 1);
					errWrite("Not Implemented: %d (Unknown command)\n", msg[1]);
					break;
			}
			break;
		default:
			break;
	}
}

void writeCmd(uint8_t cmd, uint8_t argc, uint8_t *argv) {
	uint8_t count = 0;
	uint8_t *msg;
	switch (cmd) {
		case CMD_PIN_MODE:
		case CMD_DIGITAL_PORT_DATA:
		case CMD_DIGITAL_PIN_DATA:
		case CMD_ANALOG_PIN_DATA:
			count = 3;
			msg = (uint8_t*)malloc(sizeof(uint8_t)*count);
			msg[0] = cmd;
			msg[1] = argc;
			msg[2] = *argv;
			break;
		case CMD_DIGITAL_PORT_REPORT:
		case CMD_DIGITAL_PIN_REPORT:
		case CMD_ANALOG_PIN_REPORT:
			count = 2;
			msg = (uint8_t*)malloc(sizeof(uint8_t)*count);
			msg[0] = cmd;
			msg[1] = argc;
			break;
		case CMD_SYSTEM_RESET:
		case CMD_VERSION_REPORT:
			count = 1;
			msg = (uint8_t*)malloc(sizeof(uint8_t)*count);
			msg[0] = cmd;
			break;
		case CMD_SYSEX_START:
			msg[0] = cmd;
			msg[1] = argv[0];
			for(int i=0;i<argc+2;i++) {
				msg[i+2] = argv[i];
			}
			msg[argc+1] = CMD_SYSEX_END;
			break;
		default:
			break;
	}
	binConsole->write(binConsole, argv, argc, 0);
	free(msg);
}

void runScheduler(void) {
	if (tasks) {
		long now = millis();
		task_t *current = tasks;
		task_t *previous = NULL;
		while (current) {
			if (current->ms > 0 && current->ms < now) { // TODO handle overflow
				if (execute(current)) {
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

void init(void) {
	// init hardware
	halInit();

	// setup protocol callbacks
	printString = logWrite;
	printErr = errWrite;
}

void run(void) {
	// start
	ustart = micros();
	mstart = millis();

	// --- hardware tasks
	halRun();

	// --- evaluate performance and signal lag
	if (jitter>=TICKRATE) {
		uint8_t lag[2];
		lag[0] = CMD_CONGESTION_REPORT;
		lag[1] = jitter/1000;
		writeCmd(CMD_CONGESTION_REPORT, 2, lag);
		jitter = jitter%1000;
	}

	// --- receive 1 byte
	if (binConsole->available(binConsole)) {
		binConsole->read(binConsole, &binData, 1, 0);
		if (receiveMsg(binData))
			readCmd(0, NULL);
	}
	if (txtConsole) if (txtConsole->available(txtConsole)) {
		txtConsole->read(txtConsole, &txtData, 1, 0);
		// TODO dispatch txtConsole
	}
	if (peering) if (peering->available(peering)) {
		peering->read(peering, &peerData, 1, 0);
		// TODO dispatch peerConsole
	}

	// --- scheduled tasks
	runScheduler();

	// --- evaluate spare time
	uelapsed = elapsed(mstart, ustart, micros(), millis());
	if (uelapsed >= TICKRATE) {
		jitter = jitter+(uelapsed-TICKRATE);
		return;
	}

	// --- spend spare time
	while(uelapsed<TICKRATE) {
		// receive data
		if (binConsole->available(binConsole)) {
			binConsole->read(binConsole, &binData, 1, 0);
			if (receiveMsg(binData))
				readCmd(0, NULL);
		}
		if (txtConsole) if (txtConsole->available(txtConsole)) {
			txtConsole->read(txtConsole, &txtData, 1, 0);
			// TODO dispatch txtConsole data
		}
		if (peering) if (peering->available(peering)) {
			peering->read(peering, &peerData, 1, 0);
			// TODO dispatch peering data
		}
		// evaluate elapsed time
		uelapsed = elapsed(mstart, ustart, micros(), millis());
	}
	// evaluate jitter
	jitter = jitter+(uelapsed-TICKRATE);
}
