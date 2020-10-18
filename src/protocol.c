
#include <protocol.h>
#include <stdlib.h> // for malloc

// callback functions
cbf_eval_t cbEvalEnc = NULL;
cbf_coder_t cbEncoder = NULL;
cbf_eval_t cbEvalDec = NULL;
cbf_coder_t cbDecoder = NULL;

// message handling
static uint8_t waitForData = 0;
static uint8_t waitForCRC = 0;
static uint8_t protocolErrors = 0;
uint8_t eventBuffer[PROTOCOL_MAX_EVENT_BYTES];
uint8_t eventSize = 0;
uint16_t deltaTime = 0;

// CRC-8 - Dallas/Maxim
static uint8_t CRC8(uint8_t len, const uint8_t *data) {
	uint8_t crc = 0x00;
	while (len--) {
		uint8_t extract = *data++;
		for (uint8_t tempI = 8; tempI; tempI--) {
			uint8_t sum = (crc ^ extract) & 0x01;
			crc >>= 1;
			if (sum) {
				crc ^= 0x8C;
			}
			extract >>= 1;
		}
	}
	return crc;
}

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
	waitForCRC = 0;
	eventBuffer[0] = 0;
	eventSize = 0;
}

uint8_t bufferStore(uint8_t *store) {
	for(int i=0;i<eventSize;i++) {
		store[i] = eventBuffer[i];
	}
	return eventSize;
}

void bufferLoad(uint8_t *store, uint8_t size) {
	for(int i=0;i<size;i++) {
		eventBuffer[i] = store[i];
	}
	eventSize = size;
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
	if(waitForCRC) {
		waitForCRC = 0;
		if (*byte==CRC8(size, event)) {
			// TODO cbDecode
			// signal the completed event
			return size;
		} else {
			bufferReset();
			protocolErrors++;
			return 0;
		}
	} else if(waitForData) { // need more data to complete the current event
		switch (event[1+PROTOCOL_TICK_BYTES]) {
			case STATUS_SYSEX_START:
				if (*byte == STATUS_SYSEX_END) {
					waitForData = 0;
					waitForCRC = 1;
					event[size] = *byte;
					size++;
					return 0;
				} else {
					// MSb must be 0 because those must be data bytes (ie: byte value <= 127)
					if (*byte>0x80) {
						bufferReset();
						protocolErrors++;
						return 0;
					}
					// normal data byte -> add to buffer
					event[size] = *byte;
					size++;
					return 0;
				}
				break;
			case STATUS_INTERRUPT:
			case STATUS_UNUSED_F8:
			case STATUS_UNUSED_F9:
			case STATUS_UNUSED_FA:
			case STATUS_SYSTEM_HALT:
			case STATUS_SYSTEM_RESET:
				// MSb must be 1 because those commands don't have data
				// and all bytes must be equal (transmit error detection)
				if ((*byte<0x80)||(*byte!=event[1+PROTOCOL_TICK_BYTES])) {
					bufferReset();
					protocolErrors++;
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
			case STATUS_EMERGENCY_STOP:
			case STATUS_SYSTEM_PAUSE:
			case STATUS_SYSTEM_RESUME:
				// MSb must be 0 because those events have data (ie: byte value <= 127)
				if (*byte>0x80) {
					bufferReset();
					protocolErrors++;
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
			waitForCRC = 1;
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
			case STATUS_UNUSED_F8:
			case STATUS_UNUSED_F9:
			case STATUS_UNUSED_FA:
			case STATUS_EMERGENCY_STOP:
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
				bufferReset();
				protocolErrors++;
				return 0;
				break;
		}
		event[0] = *byte;
		size = 1;
	} else if (size>0 && size<1+PROTOCOL_TICK_BYTES) {  // delta time bytes
		if (*byte>0x80) {
			bufferReset();
			protocolErrors++;
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
		case STATUS_INTERRUPT: // events without data repeat the status byte in all the 3 bytes
		case STATUS_UNUSED_F8:
		case STATUS_UNUSED_F9:
		case STATUS_UNUSED_FA:
		case STATUS_EMERGENCY_STOP:
		case STATUS_SYSTEM_HALT:
		case STATUS_SYSTEM_RESET:
			count += 4;
			event = (uint8_t*)calloc(count,sizeof(uint8_t));
			event[0] = 255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			event[start] = cmd;
			event[start+1] = cmd;
			event[start+2] = cmd;
			event[count-1] = CRC8(count,event);
			return count;
			break;
		case STATUS_PIN_MODE: // events with 1 or 2 bytes of data
		case STATUS_MICROSTAMP_REPORT:
		case STATUS_TIMESTAMP_REPORT:
		case STATUS_CONGESTION_REPORT:
		case STATUS_VERSION_REPORT:
		case STATUS_ENCODING_SWITCH:
		case STATUS_SYSTEM_PAUSE:
		case STATUS_SYSTEM_RESUME:
			count += 4;
			event = (uint8_t*)realloc(event,count*sizeof(uint8_t));
			event[0] = (uint8_t)255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			event[start] = cmd;
			event[start+1] = argc;
			event[start+2] = *argv;
			event[count-1] = CRC8(count,event);
			return count;
			break;
		case STATUS_DIGITAL_PORT_DATA: // events with channel id and 2 bytes of data
		case STATUS_DIGITAL_PIN_DATA:
		case STATUS_ANALOG_PIN_DATA:
		case STATUS_DIGITAL_PORT_REPORT:
		case STATUS_DIGITAL_PIN_REPORT:
		case STATUS_ANALOG_PIN_REPORT:
			count += 4;
			event = (uint8_t*)calloc(count,sizeof(uint8_t));
			event[0] = 255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			// MSB=status + LSB=pin or port, bits are zeroed to prevent wrong input value
			event[start] = (cmd & 0b11110000) + (argc & 0b00001111);
			event[start+1] = argv[0];
			event[start+2] = argv[1];
			event[count-1] = CRC8(count,event);
			return count;
			break;
		case STATUS_SYSEX_START: // sysex events
			datasize += cbEvalEnc(argc-2);
			count += datasize+1;
			event = (uint8_t*)calloc(count,sizeof(uint8_t));
			event[0] = 255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			event[start] = cmd; // sysex start
			event[start+1] = argv[0]; // mod byte
			event[start+2] = argv[1]; // sysex id
			cbEncoder(&argv[2], datasize, &event[start+3]);
			event[count-2] = STATUS_SYSEX_END;
			switch(argv[0]) {
				case SYSEX_MOD_EXTEND:
					// TODO call extended sysex handler
					break;
				case SYSEX_MOD_REALTIME:
					// TODO call realtime sysex handler
					break;
				default:
					// normal, non-realtime, sysex
					event[count-2] = CRC8(count-1,event);
					return count;
					break;
			}
			break;
	}
	return 0;
}

uint8_t encodeTask(task_t *task, uint8_t error, uint8_t *event) {
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

uint8_t encodePinMode(uint8_t *result, uint8_t pin, uint8_t mode) {
	return 0;
}

uint8_t encodeDigitalPortData(uint8_t *result, uint8_t port, uint8_t value) {
	return 0;
}

uint8_t encodeDigitalPinData(uint8_t *result, uint8_t pin, uint8_t value) {
	return 0;
}

uint8_t encodeAnalogPinData(uint8_t *result, uint8_t pin, uint8_t value) {
	return 0;
}

uint8_t encodeDigitalPortReport(uint8_t *result, uint8_t port, uint8_t value) {
	return 0;
}

uint8_t encodeDigitalPinReport(uint8_t *result, uint8_t pin, uint8_t value) {
	return 0;
}

uint8_t encodeAnalogPinReport(uint8_t *result, uint8_t pin, uint8_t value) {
	return 0;
}

uint8_t encodeMicrostampReport(uint8_t *result, uint32_t us) {
	return 0;
}

uint8_t encodeTimestampReport(uint8_t *result, uint16_t s) {
	return 0;
}

uint8_t encodeCongestionReport(uint8_t *result, uint8_t lag) {
	return encodeEvent(STATUS_CONGESTION_REPORT, lag, NULL, result);
}

uint8_t encodeVersionReport(uint8_t *result) {
	uint8_t byte = PROTOCOL_VERSION_MINOR;
	return encodeEvent(STATUS_VERSION_REPORT, PROTOCOL_VERSION_MAJOR, &byte, result);
}

uint8_t encodeInterrupt(uint8_t *result) {
	return 0;
}

uint8_t encodeEncodingSwitch(uint8_t *result, uint8_t proto) {
	return encodeEvent(STATUS_ENCODING_SWITCH, proto, NULL, result);
}

uint8_t encodeEmergencyStop1(uint8_t *result) {
	return 0;
}

uint8_t encodeEmergencyStop2(uint8_t *result) {
	return 0;
}

uint8_t encodeEmergencyStop3(uint8_t *result) {
	return 0;
}

uint8_t encodeEmergencyStop4(uint8_t *result) {
	return 0;
}

uint8_t encodeSystemPause(uint8_t *result, uint16_t delay) {
	return 0;
}

uint8_t encodeSystemResume(uint8_t *result, uint16_t delay) {
	return 0;
}

uint8_t encodeHalt(uint8_t *result) {
	return 0;
}

uint8_t encodeReset(uint8_t *result) {
	return 0;
}

uint8_t encodeSysex(uint8_t *result, uint8_t argc, uint8_t *argv) {
	return encodeEvent(STATUS_SYSEX_START, argc, argv, result);
}

