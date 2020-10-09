
#include <protocol/FirmataScheduler.hpp>

void delayTaskCallback(long delay) {
	fSched.delayTask(delay);
}

FirmataScheduler::FirmataScheduler() {
	tasks = NULL;
	running = NULL;
	firmata.attachDelayTask(delayTaskCallback);
}

void FirmataScheduler::handleCapability(uint8_t pin) { 
}

bool FirmataScheduler::handlePinMode(uint8_t pin, int mode) {
	return false;
}

bool FirmataScheduler::handleSysex(uint8_t command, uint8_t argc, uint8_t* argv) {
	if (command == SCHEDULER_DATA) {
		if (argc > 0) {
			switch (argv[0]) {
				case CREATE_FIRMATA_TASK:
					if (argc == 4) {
						createTask(argv[1], argv[2] | argv[3] << 7);
					}
					break;
				case DELETE_FIRMATA_TASK:
					if (argc == 2) {
						deleteTask(argv[1]);
					}
					break;
				case ADD_TO_FIRMATA_TASK:
					if (argc > 2) {
						int len = num7BitOutbytes(argc - 2);
						fEnc.read(len, argv + 2, argv + 2); //decode inplace
						addToTask(argv[1], len, argv + 2); //addToTask copies data...
					}
					break;
				case DELAY_FIRMATA_TASK:
					if (argc == 6) {
						argv++;
						fEnc.read(4, argv, argv); //decode inplace
						delayTask(*(long*)((uint8_t*)argv));
					}
					break;
				case SCHEDULE_FIRMATA_TASK:
					if (argc == 7) { //one byte taskid, 5 bytes to encode 4 bytes of long
						fEnc.read(4, argv + 2, argv + 2); //decode inplace
						schedule(argv[1], *(long*)((uint8_t*)argv + 2)); //argv[1] | argv[2]<<8 | argv[3]<<16 | argv[4]<<24
					}
					break;
				case QUERY_ALL_FIRMATA_TASKS:
					queryAllTasks();
					break;
				case QUERY_FIRMATA_TASK:
					if (argc == 2) {
						queryTask(argv[1]);
					}
					break;
				case RESET_FIRMATA_TASKS:
					reset();
			}
		}
		return true;
	}
	return false;
}

void FirmataScheduler::createTask(uint8_t id, int len) {
	firmata_task *existing = findTask(id);
	if (existing) {
		reportTask(id, existing, true);
	} else {
		firmata_task *newTask = (firmata_task*)malloc(sizeof(firmata_task) + len);
		newTask->id = id;
		newTask->time_ms = 0;
		newTask->len = len;
		newTask->nextTask = tasks;
		newTask->pos = 0;
		tasks = newTask;
	}
}

void FirmataScheduler::deleteTask(uint8_t id) {
	firmata_task *current = tasks;
	firmata_task *previous = NULL;
	while (current) {
		if (current->id == id) {
			if (previous) {
				previous->nextTask = current->nextTask;
			} else {
				tasks = current->nextTask;
			}
			free (current);
			return;
		} else {
			previous = current;
			current = current->nextTask;
		}
	}
}

void FirmataScheduler::addToTask(uint8_t id, int additionalBytes, uint8_t *message) {
	firmata_task *existing = findTask(id);
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

void FirmataScheduler::schedule(uint8_t id, long delay_ms) {
	firmata_task *existing = findTask(id);
	if (existing) {
		existing->pos = 0;
		existing->time_ms = millis() + delay_ms;
	} else {
		reportTask(id, NULL, true);
	}
}

void FirmataScheduler::delayTask(long delay_ms) {
	if (running) {
		long now = millis();
		running->time_ms += delay_ms;
		if (running->time_ms < now) { //if delay time allready passed by schedule to 'now'.
			running->time_ms = now;
		}
	}
}

void FirmataScheduler::queryAllTasks() {
	firmata.write(START_SYSEX);
	firmata.write(SCHEDULER_DATA);
	firmata.write(QUERY_ALL_TASKS_REPLY);
	firmata_task *task = tasks;
	while (task) {
		firmata.write(task->id);
		task = task->nextTask;
	}
	firmata.write(END_SYSEX);
}

void FirmataScheduler::queryTask(uint8_t id) {
	firmata_task *task = findTask(id);
	reportTask(id, task, false);
}

void FirmataScheduler::reportTask(uint8_t id, firmata_task *task, bool error) {
	firmata.write(START_SYSEX);
	firmata.write(SCHEDULER_DATA);
	if (error) {
		firmata.write(ERROR_TASK_REPLY);
	} else {
		firmata.write(QUERY_TASK_REPLY);
	}
	firmata.write(id);
	if (task) {
		fEnc.start();
		for (unsigned long i = 3; i < firmata_task_len(task); i++) {
			fEnc.write(((uint8_t *)task)[i]); //don't write first 3 bytes (firmata_task*, byte); makes use of AVR byteorder (LSB first)
		}
		fEnc.end();
	}
	firmata.write(END_SYSEX);
}

void FirmataScheduler::runTasks() {
	if (tasks) {
		long now = millis();
		firmata_task *current = tasks;
		firmata_task *previous = NULL;
		while (current) {
			if (current->time_ms > 0 && current->time_ms < now) { // TODO handle overflow
				if (execute(current)) {
					previous = current;
					current = current->nextTask;
				} else {
					if (previous) {
						previous->nextTask = current->nextTask;
						free(current);
						current = previous->nextTask;
					} else {
						tasks = current->nextTask;
						free(current);
						current = tasks;
					}
				}
			} else {
				current = current->nextTask;
			}
		}
	}
}

void FirmataScheduler::reset() {
	while (tasks) {
		firmata_task *nextTask = tasks->nextTask;
		free(tasks);
		tasks = nextTask;
	}
}

//private
bool FirmataScheduler::execute(firmata_task *task) {
	long start = task->time_ms;
	int pos = task->pos;
	int len = task->len;
	uint8_t *messages = task->messages;
	running = task;
	while (pos < len) {
		firmata.parse(messages[pos++]);
		if (start != task->time_ms) { // return true if task got rescheduled during run.
			task->pos = ( pos == len ? 0 : pos ); // last message executed? -> start over next time
			running = NULL;
			return true;
		}
	}
	running = NULL;
	return false;
}

firmata_task *FirmataScheduler::findTask(uint8_t id) {
	firmata_task *currentTask = tasks;
	while (currentTask) {
		if (id == currentTask->id) {
			return currentTask;
		} else {
			currentTask = currentTask->nextTask;
		}
	}
	return NULL;
}

