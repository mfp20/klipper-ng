
#include <protocol.h>
#include <stdlib.h> // for malloc

// callback functions
cbf_varg_t printString = NULL;
cbf_varg_t printErr = NULL;
cbf_eval_t cbEvalEnc = NULL;
cbf_coder_t cbEncoder = NULL;
cbf_eval_t cbEvalDec = NULL;
cbf_coder_t cbDecoder = NULL;
cbf_data_t runEvent = NULL;

// message handling
static uint8_t waitForData = 0;
static uint8_t eventBuffer[PROTOCOL_MAX_EVENT_BYTES]; // current event in transit
static uint8_t eventSize = 0; // current event size
uint16_t deltaTime = 0;

// tasks handling
task_t *tasks = NULL;
task_t *running = NULL;

static uint8_t evalEncode(uint8_t count) {
	return 0;
}

static uint8_t evalDecode(uint8_t count) {
	return 0;
}

static void encode7bit(uint8_t *input, uint8_t count, uint8_t *output) {
	uint8_t i = 0;
	for (uint8_t j=0;j<count;j++) {
		output[i] = (input[j] & 0b01111111); // LSB, to use 0bXXXXXXXX notation enable -std=gnu++17 gcc extensions or similarly on clang
		output[i+1] = (input[j] >> 7 & 0b01111111); // MSB
		i = i+2;
	}
}

static void decode7bit(uint8_t *input, uint8_t count, uint8_t *output) {
	uint8_t bufferLength = (count - 1) / 2;
	uint8_t i = 1;
	uint8_t j = 0;
	while (j < bufferLength) {
		output[j] = input[i];
		i++;
		output[j] += (input[i] << 7);
		i++;
		j++;
	}
}

static uint8_t evalEncodeCompat(uint8_t count) {
	return 0;
}

static uint8_t evalDecodeCompat(uint8_t count) {
	return 0;
}

static void encode7bitCompat(uint8_t *input, uint8_t count, uint8_t *output) {
	uint8_t previous = 0;
	uint8_t shift = 0;
	uint8_t i = 0;
	for(uint8_t j=0;j<count;j++) {
		if (shift == 0) {
			output[i] = (input[j] & 0x7f);
			i++;
			shift++;
			previous = input[j] >> 7;
		} else {
			output[i] = (((input[j] << shift) & 0x7f) | previous);
			i++;
			if (shift == 6) {
				output[i] = (input[j] >> 1);
				i++;
				shift = 0;
			} else {
				shift++;
				previous = input[j] >> (8 - shift);
			}
		}
	}
	if (shift > 0) {
		i++;
		output[i] = (previous);
	}
}

static void decode7bitCompat(uint8_t *input, uint8_t count, uint8_t *output) {
	for (uint8_t i = 0; i < count; i++) {
		uint8_t j = i << 3;
		uint8_t pos = j / 7;
		uint8_t shift = j % 7;
		output[i] = (input[pos] >> shift) | ((output[pos + 1] << (7 - shift)) & 0xFF);
	}
}

void bufferReset(void) {
	waitForData = 0;
	eventBuffer[0] = 0;
	eventSize = 0;
}

uint8_t bufferCopy(uint8_t *event) {
	for(int i=0;i<eventSize;i++) {
		event[i] = eventBuffer[i];
	}
	return eventSize;
}

uint8_t encodingSwitch(uint8_t proto) {
	switch(proto) {
		case PROTOCOL_ENCODING_REPORT:
			if (cbEvalEnc==evalEncode) {
				return (uint8_t)PROTOCOL_ENCODING_NORMAL;
			} else {
				return (uint8_t)PROTOCOL_ENCODING_COMPAT;
			}
			break;
		case PROTOCOL_ENCODING_NORMAL:
			cbEvalEnc = evalEncode;
			cbEncoder = encode7bit;
			cbEvalDec = evalDecode;
			cbDecoder = decode7bit;
			break;
		case PROTOCOL_ENCODING_COMPAT:
			cbEvalEnc = evalEncodeCompat;
			cbEncoder = encode7bitCompat;
			cbEvalDec = evalDecodeCompat;
			cbDecoder = decode7bitCompat;
			break;
	}
	return proto;
}

uint8_t decodeEvent(uint8_t *byte, uint8_t size, uint8_t *event) {
	if (event == NULL) {
		size = eventSize;
		event = eventBuffer;
	}
	if(waitForData) { // need more data to complete the current event
		switch (event[1+PROTOCOL_TICK_BYTES]) {
			case STATUS_SYSEX_START:
				if (*byte == STATUS_SYSEX_END) {
					waitForData = 0;
					event[size] = *byte;
					size++;
					// TODO cbDecode
					// signal the completed event
					return size;
				} else {
					// MSb must be 0 because those must be data bytes (ie: byte value <= 127)
					if (*byte>0x80) {
						// TODO signal transmit error
						bufferReset();
						return 0;
					}
					// normal data byte -> add to buffer
					event[size] = *byte;
					size++;
					return 0;
				}
				break;
			case STATUS_INTERRUPT:
			case STATUS_EMERGENCY_STOP1:
			case STATUS_EMERGENCY_STOP2:
			case STATUS_EMERGENCY_STOP3:
			case STATUS_EMERGENCY_STOP4:
			case STATUS_SYSTEM_HALT:
			case STATUS_SYSTEM_RESET:
				// MSb must be 1 because those commands don't have data
				// and all bytes must be equal (transmit error detection)
				if ((*byte<0x80)||(*byte!=event[1+PROTOCOL_TICK_BYTES])) {
					// TODO signal transmit error
					bufferReset();
					return 0;
				}
				break;
			case STATUS_PIN_MODE:
			case STATUS_DIGITAL_PORT_DATA:
			case STATUS_DIGITAL_PIN_DATA:
			case STATUS_ANALOG_PIN_DATA:
			case STATUS_DIGITAL_PORT_REPORT:
			case STATUS_DIGITAL_PIN_REPORT:
			case STATUS_ANALOG_PIN_REPORT:
			case STATUS_MICROSTAMP_REPORT:
			case STATUS_TIMESTAMP_REPORT:
			case STATUS_CONGESTION_REPORT:
			case STATUS_VERSION_REPORT:
			case STATUS_ENCODING_SWITCH:
			case STATUS_SYSTEM_PAUSE:
			case STATUS_SYSTEM_RESUME:
				// MSb must be 0 because those events have data (ie: byte value <= 127)
				if (*byte>0x80) {
					// TODO signal transmit error
					bufferReset();
					return 0;
				}
				break;
			default:
				break;
		}
		// add current byte to buffer
		waitForData--;
		event[size] = *byte;
		size++;
		if (waitForData == 0) { // got the whole event
			// signal the completed event
			return size;
		}
	} else if (size==1+PROTOCOL_TICK_BYTES) { // status bytes start
		switch (event[1+PROTOCOL_TICK_BYTES]) {
			case STATUS_PIN_MODE:
			case STATUS_DIGITAL_PORT_DATA:
			case STATUS_DIGITAL_PIN_DATA:
			case STATUS_ANALOG_PIN_DATA:
			case STATUS_DIGITAL_PORT_REPORT:
			case STATUS_DIGITAL_PIN_REPORT:
			case STATUS_ANALOG_PIN_REPORT:
			case STATUS_MICROSTAMP_REPORT:
			case STATUS_TIMESTAMP_REPORT:
			case STATUS_CONGESTION_REPORT:
			case STATUS_VERSION_REPORT:
			case STATUS_INTERRUPT:
			case STATUS_ENCODING_SWITCH:
			case STATUS_EMERGENCY_STOP1:
			case STATUS_EMERGENCY_STOP2:
			case STATUS_EMERGENCY_STOP3:
			case STATUS_EMERGENCY_STOP4:
			case STATUS_SYSTEM_PAUSE:
			case STATUS_SYSTEM_RESUME:
			case STATUS_SYSTEM_HALT:
			case STATUS_SYSTEM_RESET:
				waitForData = 2; // two more bytes needed
				break;
			case STATUS_SYSEX_START:
				waitForData = 1; // more data needed
				break;
			default:
				// TODO signal error
				bufferReset();
				return 0;
				break;
		}
		event[0] = *byte;
		size = 1;
	} else if (size>0 && size<1+PROTOCOL_TICK_BYTES) {  // delta time bytes
		if (*byte>0x80) {
			// TODO signal error
			bufferReset();
			return 0;
		}
		event[size]=*byte;
		size++;
	} else { // new event start
		if (*byte == 255) {
			size = 0;
			event[size] = *byte;
			size++;
		}
	}
	return 0;
}

uint8_t encodeEvent(uint8_t cmd, uint8_t argc, uint8_t *argv, uint8_t *event) {
	uint8_t count = 1+PROTOCOL_TICK_BYTES;
	uint8_t start = count;
	uint8_t datasize;
	switch (cmd) {
		case STATUS_ENCODING_SWITCH: //
			break;
			// events without data repeat the status byte in all the 3 bytes
		case STATUS_INTERRUPT:
		case STATUS_EMERGENCY_STOP1:
		case STATUS_EMERGENCY_STOP2:
		case STATUS_EMERGENCY_STOP3:
		case STATUS_EMERGENCY_STOP4:
		case STATUS_SYSTEM_HALT:
		case STATUS_SYSTEM_RESET:
			count += 3;
			event = (uint8_t*)calloc(count,sizeof(uint8_t));
			event[0] = 255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			event[start] = cmd;
			event[start+1] = cmd;
			event[start+2] = cmd;
			return count;
			break;
		case STATUS_PIN_MODE:
		case STATUS_MICROSTAMP_REPORT:
		case STATUS_TIMESTAMP_REPORT:
		case STATUS_CONGESTION_REPORT:
		case STATUS_VERSION_REPORT:
		case STATUS_SYSTEM_PAUSE:
		case STATUS_SYSTEM_RESUME:
			count += 3;
			event = (uint8_t*)realloc(event,count*sizeof(uint8_t));
			event[0] = (uint8_t)255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			event[start] = cmd;
			event[start+1] = argc;
			event[start+2] = *argv;
			return count;
			break;
		case STATUS_DIGITAL_PORT_DATA:
		case STATUS_DIGITAL_PIN_DATA:
		case STATUS_ANALOG_PIN_DATA:
		case STATUS_DIGITAL_PORT_REPORT:
		case STATUS_DIGITAL_PIN_REPORT:
		case STATUS_ANALOG_PIN_REPORT:
			count += 3;
			event = (uint8_t*)calloc(count,sizeof(uint8_t));
			event[0] = 255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			// MSB=status + LSB=pin or port, bits are zeroed to prevent wrong input value
			event[start] = (cmd & 0b11110000) + (argc & 0b00001111);
			event[start+1] = argv[0];
			event[start+2] = argv[1];
			return count;
			break;
		case STATUS_SYSEX_START:
			switch(argv[0]) {
				case SYSEX_MOD_EXTEND:
					count += 4;
					event = (uint8_t*)calloc(count,sizeof(uint8_t));
					event[0] = 255;
					event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
					event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
					event[start] = cmd; // sysex start
					event[start+1] = argv[0]; // mod byte
					event[start+2] = argv[1]; // extended sysex byte
					event[start+3] = STATUS_SYSEX_END;
					return count;
					break;
				case SYSEX_MOD_REALTIME:
					count += 3;
					event = (uint8_t*)calloc(count,sizeof(uint8_t));
					event[0] = 255;
					event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
					event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
					event[start] = cmd; // sysex start
					event[start+1] = argv[0]; // mod byte
					// no data bytes, the system will go in realtime mode
					event[start+2] = STATUS_SYSEX_END;
					return count;
					break;
				default:
					datasize += cbEvalEnc(argc-2);
					count += datasize;
					event = (uint8_t*)calloc(count,sizeof(uint8_t));
					event[0] = 255;
					event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
					event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
					event[start] = cmd; // sysex start
					event[start+1] = argv[0]; // mod byte
					event[start+2] = argv[1]; // sysex id
					cbEncoder(&argv[2], datasize, &event[start+3]);
					event[count-1] = STATUS_SYSEX_END;
					return count;
					break;
			}
			break;
	}
	return 0;
}

void printEvent(uint8_t size, uint8_t *event) {
	if (event == NULL) {
		size = eventSize;
		event = eventBuffer;
	}
	switch(event[0]) {
		case STATUS_SYSEX_START:
			printString("%02x ", event[1]);
			for(uint8_t i=1;i<size;i++) {
				printString("%02x ", event[i]);
			}
			printString("(");
			switch(event[1]) {
				case SYSEX_MOD_EXTEND:
					break;
				case SYSEX_RCSWITCH_OUT:
					printString("Not implemented");
					break;
				case SYSEX_RCSWITCH_IN:
					printString("Not implemented");
					break;
				case SYSEX_DEVICE_REQ:
					printString("Not implemented");
					break;
				case SYSEX_DEVICE_REP:
					printString("Not implemented");
					break;
				case SYSEX_UART_DATA:
					printString("Not implemented");
					break;
				case SYSEX_FEATURES_REPORT:
					printString("Not implemented");
					break;
				case SYSEX_SPI_DATA:
					printString("Not implemented");
					break;
				case SYSEX_PINMAP_REQ:
					printString("Not implemented");
					break;
				case SYSEX_PINMAP_REP:
					printString("Not implemented");
					break;
				case SYSEX_PINCAPS_REQ:
					printString("Not implemented");
					break;
				case SYSEX_PINCAPS_REP:
					printString("Not implemented");
					break;
				case SYSEX_PINSTATE_REQ:
					printString("Not implemented");
					break;
				case SYSEX_PINSTATE_REP:
					printString("Not implemented");
					break;
				case SYSEX_STRING_DATA:
					printString("Not implemented");
					break;
				case SYSEX_ONEWIRE_DATA:
					printString("Not implemented");
					break;
				case SYSEX_I2C_DATA:
					printString("Not implemented");
					break;
				case SYSEX_VERSION_REPORT:
					printString("Not implemented");
					break;
				case SYSEX_SCHEDULER_DATA:
					printString("Not implemented");
					break;
				case SYSEX_MOD_NON_REALTIME:
					printString("Not implemented");
					break;
				case SYSEX_MOD_REALTIME:
					printString("Not implemented");
					break;
				default:
					printString("Unknown SysEx type %#x", event[1]);
					break;
			}
			printString(")");
			break;
		case STATUS_PIN_MODE:
		case STATUS_DIGITAL_PORT_DATA:
		case STATUS_DIGITAL_PIN_DATA:
		case STATUS_ANALOG_PIN_DATA:
			printString("%02x %02x %02x\n", event[3], event[4], event[5]);
			break;
		case STATUS_DIGITAL_PORT_REPORT:
		case STATUS_DIGITAL_PIN_REPORT:
		case STATUS_ANALOG_PIN_REPORT:
			printString("%02x %02x\n", event[3], event[4]);
			break;
		case STATUS_SYSTEM_RESET:
		case STATUS_VERSION_REPORT:
			printString("%02x\n", event[3]);
			break;
		default:
			printString("NULL\n");
			break;
	}
}

uint8_t encodeTask(task_t *task, bool error, uint8_t *event) {
	uint8_t tasklen;
	uint8_t *data;
	if (error) {
		tasklen = 0;
		data = (uint8_t*)calloc(3,sizeof(uint8_t));
		data[0] = SYSEX_MOD_NON_REALTIME;
		data[1] = SYSEX_SCHEDULER_DATA;
		data[2] = SYSEX_SUB_SCHED_ERROR_REP;
	} else {
		tasklen = sizeof(task_t)+task->len;
		data = (uint8_t*)calloc(3+tasklen,sizeof(uint8_t));
		data[0] = SYSEX_MOD_NON_REALTIME;
		data[1] = SYSEX_SCHEDULER_DATA;
		data[2] = SYSEX_SUB_SCHED_TASK_REP;
		// dump all task in data
		for (uint8_t i=0;i<tasklen;++i) {
			data[i] = ((uint8_t *)task)[i];
		}
	}
	return encodeEvent(STATUS_SYSEX_START, 3+tasklen, data, event);
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
				bool reschedule = false;
				running = current;
				while (pos < len) {
					if (decodeEvent(&messages[pos++], 0, NULL))
						runEvent(eventSize, eventBuffer);
					if (start != current->ms) { // return true if task got rescheduled during run.
						current->pos = ( pos == len ? 0 : pos ); // last message executed? -> start over next time
						reschedule = true;
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

