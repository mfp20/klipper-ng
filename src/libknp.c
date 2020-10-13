
#include <libknp.h>

uint16_t mstart = 0;
uint16_t ustart = 0;
uint16_t uelapsed = 1000;
uint16_t jitter = 0;
uint8_t txtData, binData, peerData;

task_t *tasks = NULL;
task_t *running = NULL;

static bool execute(task_t *task) {
	long start = task->ms;
	int pos = task->pos;
	int len = task->len;
	uint8_t *messages = task->messages;
	running = task;
	while (pos < len) {
		if (collect(messages[pos++]))
			dispatchMsg(NULL);
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
	uint8_t byte = CMD_START_SYSEX;
	binConsole->write(binConsole, &byte, 1, 0);
	byte = CMD_EID_SCHEDULER_DATA;
	binConsole->write(binConsole, &byte, 1, 0);
	if (error) {
		byte = CMD_SUB_SCHED_REPLY_ERROR;
		binConsole->write(binConsole, &byte, 1, 0);
	} else {
		byte = CMD_SUB_SCHED_REPLY_QUERY_TASK;
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
	byte = CMD_END_SYSEX;
	binConsole->write(binConsole, &byte, 1, 0);
}

void cmdReportProtocolVersion(void) {
	errWrite("Not Implemented: cmdReportProtocolVersion");
}

void cmdReportFirmwareVersion(void) {
	errWrite("Not Implemented: cmdReportFirmwareVersion");
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
	uint8_t byte = CMD_START_SYSEX;
	binConsole->write(binConsole, &byte, 1, 0);
	byte = CMD_EID_SCHEDULER_DATA;
	binConsole->write(binConsole, &byte, 1, 0);
	byte = CMD_SUB_SCHED_REPLY_QUERY_ALL;
	binConsole->write(binConsole, &byte, 1, 0);
	task_t *task = tasks;
	while (task) {
		binConsole->write(binConsole, &task->id, 1, 0);
		task = task->next;
	}
	byte = CMD_END_SYSEX;
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

// initialize a default state
void cmdSystemReset(void) {
	// pins with analog capability default to analog input
	// otherwise, pins default to digital output
	for (uint8_t i = 0; i < TOTAL_PINS; i++) {
		if (IS_PIN_ANALOG(i)) {
		} else if (IS_PIN_DIGITAL(i)) {
		}
	}
	// reset pin_status
	free(pin_status);
	pin_status_size = 0;
}

void cmdRealtimeServer(uint16_t timeout) {
	bool loop = true;
	uint8_t query = 0;
	uint8_t answer = CMD_EID_NON_REALTIME;
	uint8_t data = 0;
	uint16_t mstart, ustart, uend, mend;
	if (timeout > 999) timeout = 999;
	mstart = millis();
	while(loop) {
		ustart = micros();
		binConsole->read(binConsole, &query, 1, timeout);
		switch (query) {
			case CMD_REALTIME_SYN:
				answer = CMD_REALTIME_ACK;
				binConsole->write(binConsole, &answer, 1, 0);
				break;
			case CMD_REALTIME_ACK:
				answer = micros(); // TODO get micros MSB only;
				binConsole->write(binConsole, &answer, 1, 0);
				binConsole->read(binConsole, &data, 1, timeout);
				break;
			case CMD_REALTIME_NACK:
				answer = 1; // TODO error code
				binConsole->write(binConsole, &answer, 1, 0);
				break;
			case CMD_REALTIME_TIME_RESET: // sync microseconds
				//hal_time_reset();
				answer = micros(); // TODO get micros MSB only;
				binConsole->write(binConsole, &answer, 1, 1);
				binConsole->read(binConsole, &data, 1, timeout); // receive offset
				//hal_time_adj(data);
				break;
			case CMD_REALTIME_FIN:
				answer = CMD_EID_NON_REALTIME;
				binConsole->write(binConsole, &answer, 1, 0);
				break;
			case CMD_EID_NON_REALTIME: // exit 'realtime mode'
			default:
				answer = CMD_EID_NON_REALTIME;
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
	uint8_t query = CMD_REALTIME_ACK;
	uint8_t data = 0;
	uint16_t mstart, ustart, uend, mend;
	uint16_t timeout = timeout;
	if (timeout > 999) timeout = 999;
	mstart = millis();
	ustart = micros();
	switch (rpc) {
		case CMD_REALTIME_ACK: // ping-pong to measure RR time
			// TODO
			break;
		case CMD_REALTIME_TIME_RESET: // sync microseconds
			// TODO
			break;
		case CMD_EID_NON_REALTIME: // exit 'realtime mode'
		default:
			query = CMD_EID_NON_REALTIME;
			binConsole->write(binConsole, &query, 1, 0);
			break;
	}
	return data;
}

void dispatchSimple(uint8_t cmd, uint8_t byte1, uint8_t byte2) {
	switch (cmd) {
		case CMD_SET_PIN_MODE:
			errWrite("Not Implemented: %d (CMD_SET_PIN_MODE)", cmd);
			break;
		case CMD_SET_DIGITAL_PIN_VALUE:
			errWrite("Not Implemented: %d (CMD_SET_DIGITAL_PIN_VALUE)", cmd);
			break;
		case CMD_DIGITAL_MESSAGE:
			errWrite("Not Implemented: %d (CMD_DIGITAL_MESSAGE)", cmd);
			break;
		case CMD_ANALOG_MESSAGE:
			errWrite("Not Implemented: %d (CMD_ANALOG_MESSAGE)", cmd);
			break;
		case CMD_REPORT_ANALOG:
			errWrite("Not Implemented: %d (CMD_REPORT_ANALOG)", cmd);
			break;
		case CMD_REPORT_DIGITAL:
			errWrite("Not Implemented: %d (CMD_REPORT_DIGITAL)", cmd);
			break;
		case CMD_REPORT_VERSION:
			cmdReportProtocolVersion();
			break;
		case CMD_SYSTEM_RESET:
			cmdSystemReset();
			break;
		default:
			break;
	}
}

void dispatchSysex(uint8_t cmd, uint8_t argc, uint8_t *argv) {
	uint8_t datalen = 0;
	switch(cmd) {
		case CMD_EID_RCOUT:
			errWrite("Not Implemented: %d (CMD_EID_RCOUT)", cmd);
			break;
		case CMD_EID_RCIN:
			errWrite("Not Implemented: %d (CMD_EID_RCIN)", cmd);
			break;
		case CMD_EID_DEVICE_QUERY:
			errWrite("Not Implemented: %d (CMD_EID_DEVICE_QUERY)", cmd);
			break;
		case CMD_EID_DEVICE_RESPONSE:
			errWrite("Not Implemented: %d (CMD_EID_DEVICE_RESPONSE)", cmd);
			break;
		case CMD_EID_SERIAL_MESSAGE:
			errWrite("Not Implemented: %d (CMD_EID_SERIAL_MESSAGE)", cmd);
			break;
		case CMD_EID_ENCODER_DATA:
			errWrite("Not Implemented: %d (CMD_EID_ENCODER_DATA)", cmd);
			break;
		case CMD_EID_ACCELSTEPPER_DATA:
			errWrite("Not Implemented: %d (CMD_EID_ACCELSTEPPER_DATA)", cmd);
			break;
		case CMD_EID_REPORT_DIGITAL_PIN:
			errWrite("Not Implemented: %d (CMD_EID_REPORT_DIGITAL_PIN)", cmd);
			break;
		case CMD_EID_EXTENDED_REPORT_ANALOG:
			errWrite("Not Implemented: %d (CMD_EID_EXTENDED_REPORT_ANALOG)", cmd);
			break;
		case CMD_EID_REPORT_FEATURES:
			errWrite("Not Implemented: %d (CMD_EID_REPORT_FEATURES)", cmd);
			break;
		case CMD_EID_SERIAL_DATA2:
			errWrite("Not Implemented: %d (CMD_EID_SERIAL_DATA2)", cmd);
			break;
		case CMD_EID_SPI_DATA:
			errWrite("Not Implemented: %d (CMD_EID_SPI_DATA)", cmd);
			break;
		case CMD_EID_ANALOG_MAPPING_QUERY:
			errWrite("Not Implemented: %d (CMD_EID_ANALOG_MAPPING_QUERY)", cmd);
			break;
		case CMD_EID_ANALOG_MAPPING_RESPONSE:
			errWrite("Not Implemented: %d (CMD_EID_ANALOG_MAPPING_RESPONSE)", cmd);
			break;
		case CMD_EID_CAPABILITY_QUERY:
			errWrite("Not Implemented: %d (CMD_EID_CAPABILITY_QUERY)", cmd);
			break;
		case CMD_EID_CAPABILITY_RESPONSE:
			errWrite("Not Implemented: %d (CMD_EID_CAPABILITY_RESPONSE)", cmd);
			break;
		case CMD_EID_PIN_STATE_QUERY:
			errWrite("Not Implemented: %d (CMD_EID_PIN_STATE_QUERY)", cmd);
			break;
		case CMD_EID_PIN_STATE_RESPONSE:
			errWrite("Not Implemented: %d (CMD_EID_PIN_STATE_RESPONSE)", cmd);
			break;
		case CMD_EID_EXTENDED_ANALOG:
			errWrite("Not Implemented: %d (CMD_EID_EXTENDED_ANALOG)", cmd);
			break;
		case CMD_EID_SERVO_CONFIG:
			errWrite("Not Implemented: %d (CMD_EID_SERVO_CONFIG)", cmd);
			break;
		case CMD_EID_STRING_DATA:
			// The string length will only be at most half the size of the
			// stored input buffer so we can decode the string within the buffer.
			datalen = (argc-1)/2;
			cbDecoder(argv, datalen, argv);
			// Make sure string is null terminated. This may be the case for data
			// coming from client libraries in languages that don't null terminate
			// strings.
			if (argv[datalen-1]!='\0') argv[datalen] = '\0';
			logWrite((char *)argv);
			break;
		case CMD_EID_STEPPER_DATA:
			errWrite("Not Implemented: %d (CMD_EID_STEPPER_DATA)", cmd);
			break;
		case CMD_EID_ONEWIRE_DATA:
			errWrite("Not Implemented: %d (CMD_EID_ONEWIRE_DATA)", cmd);
			break;
		case CMD_EID_SHIFT_DATA:
			errWrite("Not Implemented: %d (CMD_EID_SHIFT_DATA)", cmd);
			break;
		case CMD_EID_I2C_REQUEST:
			errWrite("Not Implemented: %d (CMD_EID_I2C_REQUEST)", cmd);
			break;
		case CMD_EID_I2C_REPLY:
			errWrite("Not Implemented: %d (CMD_EID_I2C_REPLY)", cmd);
			break;
		case CMD_EID_I2C_CONFIG:
			errWrite("Not Implemented: %d (CMD_EID_I2C_CONFIG)", cmd);
			break;
		case CMD_EID_REPORT_FIRMWARE:
			cmdReportFirmwareVersion();
			break;
		case CMD_EID_SAMPLING_INTERVAL:
			errWrite("Not Implemented: %d (CMD_EID_SAMPLING_INTERVAL)", cmd);
			break;
		case CMD_EID_SCHEDULER_DATA:
			if (argc > 0) {
				switch (argv[0]) {
					case CMD_SUB_SCHED_CREATE:
						if (argc == 4) {
							cmdCreateTask(argv[1], argv[2] | argv[3] << 7);
						}
						break;
					case CMD_SUB_SCHED_ADD_TO:
						if (argc > 2) {
							int len = num7BitOutbytes(argc - 2);
							decode7bitCompat(argv + 2, len, argv + 2); // decode inplace
							cmdAddToTask(argv[1], len, argv + 2); // addToTask copies data...
						}
						break;
					case CMD_SUB_SCHED_DELAY:
						if (argc == 6) {
							argv++;
							decode7bitCompat(argv, 4, argv); // decode inplace
							cmdDelayTask(*(long*)((uint8_t*)argv));
						}
						break;
					case CMD_SUB_SCHED_SCHEDULE:
						if (argc == 7) { // one byte taskid, 5 bytes to encode 4 bytes of long
							decode7bitCompat(argv + 2, 4, argv + 2); // decode inplace
							cmdScheduleTask(argv[1], *(long*)((uint8_t*)argv + 2)); // argv[1] | argv[2]<<8 | argv[3]<<16 | argv[4]<<24
						}
						break;
					case CMD_SUB_SCHED_QUERY_ALL:
						cmdQueryAllTasks();
						break;
					case CMD_SUB_SCHED_QUERY_TASK:
						if (argc == 2) {
							cmdQueryTask(argv[1]);
						}
						break;
					case CMD_SUB_SCHED_DELETE:
						if (argc == 2) {
							cmdDeleteTask(argv[1]);
						}
						break;
					case CMD_SUB_SCHED_RESET:
						cmdSchedulerReset();
				}
			}
			break;
		case CMD_EID_ANALOG_CONFIG:
			errWrite("Not Implemented: %d (CMD_EID_ANALOG_CONFIG)", cmd);
			break;
		case CMD_EID_REALTIME:
			cmdRealtimeServer(TICKRATE);
			break;
		case CMD_EID_EXTEND:
			errWrite("Not Implemented: %d (CMD_EID_EXTEND)", cmd);
			break;
		default:
			errWrite("Not Implemented: %d (Unknown command)", cmd);
			break;
	}
}

void signal(uint8_t sig, uint8_t count, uint8_t *data) {
	uint8_t cmd[count+3];
	cmd[0] = CMD_START_SYSEX;
	cmd[1] = sig;
	for(int i=0;i<count;i++) {
		cmd[i+2] = data[i];
	}
	cmd[count+2] = CMD_END_SYSEX;
	logWrite(">> ");
	printMsg(count+3, cmd);
	logWrite("\n");
	binConsole->write(binConsole, cmd, count+3, 0);
}

void scheduler_run(void) {
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
	hal_init();
	// setup dispatch callbacks
	printString = logWrite;
	printErr = errWrite;
	cbSimple = dispatchSimple;
	cbSysex = dispatchSysex;
}

void run(void) {
	// start
	ustart = micros();
	mstart = millis();
	// --- hardware tasks
	hal_run();
	// --- receive 1 byte
	if (binConsole->available(binConsole)) {
		binConsole->read(binConsole, &binData, 1, 0);
		if (collect(binData))
			dispatchMsg(NULL);
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
	scheduler_run();

	// evaluate spare time
	uelapsed = elapsed(mstart, ustart, micros(), millis());
	if (uelapsed >= TICKRATE) {
		jitter = jitter+(uelapsed-TICKRATE);
		if (jitter>=TICKRATE) {
			uint8_t lag = jitter/1000;
			signal(CMD_EID_USER_SIGLAG,1,&lag);
			jitter = jitter%1000;
		}
		return;
	}

	// spend spare time
	while(uelapsed<TICKRATE) {
		// receive data
		if (binConsole->available(binConsole)) {
			binConsole->read(binConsole, &binData, 1, 0);
			if (collect(binData))
				dispatchMsg(NULL);
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
	if (jitter>=TICKRATE) {
		uint8_t lag = jitter/1000;
		signal(CMD_EID_USER_SIGLAG,1,&lag);
		jitter = jitter%1000;
	}
}
