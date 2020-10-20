
#include <protocol.h>
#include <stdlib.h> // for malloc
#include <stdio.h>	// sprintf

uint8_t pins[16];
uint8_t group[4][16];

// message handling
static uint8_t waitForData = 0;
static uint8_t waitForCRC = 0;
uint8_t eventBuffer[PROTOCOL_MAX_EVENT_BYTES];
uint8_t eventSize = 0;
uint8_t sequenceId = 0;
uint8_t protocolDebug = 1;
uint8_t protocolErrors[PROTOCOL_ERR_TOTAL];
uint8_t protocolErrorNo = 0;

// callback functions
cbf_eval_t cbEvalEnc = NULL;
cbf_coder_t cbEncoder = NULL;
cbf_eval_t cbEvalDec = NULL;
cbf_coder_t cbDecoder = NULL;
cbf_data_t customHandler = NULL;
cbf_signal_t protocolSignal = NULL;

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
		case PROTOCOL_ENCODING_REPORT:
			if (cbEvalEnc==evalEncode) {
				return (uint8_t)PROTOCOL_ENCODING_NORMAL;
			} else if (cbEvalEnc==evalEncodeCompat) {
				return (uint8_t)PROTOCOL_ENCODING_COMPAT;
			} else {
				return (uint8_t)PROTOCOL_ENCODING_UNKNOWN;
			}
			break;
		default:
				return (uint8_t)PROTOCOL_ENCODING_UNKNOWN;
			break;
	}
	return proto;
}

void decodeErr(uint8_t code) {
	bufferReset();
	if (protocolDebug) {
		protocolErrorNo++;
		protocolErrors[code]++;
		if (protocolSignal) protocolSignal(SIG_DISCARD,sequenceId);
	}
}

uint8_t decodeEvent(uint8_t *byte, uint8_t size, uint8_t *event) {
	if (event == NULL) {
		size = eventSize;
		event = eventBuffer;
	}
	if(waitForCRC) { // event complete but CRC byte missing
		if (size<6) { // size must be bigger than minimum event size
			decodeErr(PROTOCOL_ERR_SIZE);
			return 0;
		}
		waitForCRC = 0;
		if (*byte==CRC8(size, event)) {
			if (event[3] == STATUS_SYSEX_START) {
				cbDecoder(&event[3], size-2, event); // decode data bytes in place
			}
			// completed event goes live!
			return size;
		} else {
			decodeErr(PROTOCOL_ERR_CRC);
			return 0;
		}
	} else if(waitForData) { // need more data to complete the current event
		switch (event[2]) {
			case STATUS_SYSEX_START:
				if (size>3) { // size must be bigger than sequenceId header size
					decodeErr(PROTOCOL_ERR_SIZE);
					return 0;
				}
				if (*byte == STATUS_SYSEX_END) {
					waitForData = 0;
					waitForCRC = 1;
					event[size] = *byte;
					size++;
					return 0;
				} else {
					// MSb must be 0 because those must be data bytes (ie: byte value <= 127)
					if (*byte>0x80) {
						decodeErr(PROTOCOL_ERR_NEED_DATA);
						return 0;
					}
					// normal data byte -> add to buffer
					event[size] = *byte;
					size++;
					return 0;
				}
				break;
			case STATUS_PIN_MODE:
			case STATUS_DIGITAL_PORT_REPORT:
			case STATUS_DIGITAL_PORT_SET:
			case STATUS_DIGITAL_PIN_REPORT:
			case STATUS_DIGITAL_PIN_SET:
			case STATUS_ANALOG_PIN_REPORT:
			case STATUS_ANALOG_PIN_SET:
			case STATUS_PROTOCOL_VERSION:
			case STATUS_PROTOCOL_ENCODING:
			case STATUS_INFO:
			case STATUS_SIGNAL:
			case STATUS_INTERRUPT:
			case STATUS_EMERGENCY_STOP:
			case STATUS_SYSTEM_PAUSE:
			case STATUS_SYSTEM_RESUME:
			case STATUS_SYSTEM_RESET:
				// MSb must be 0 because those events have data (ie: byte value <= 127)
				if (*byte>0x80) {
					decodeErr(PROTOCOL_ERR_NEED_DATA);
					return 0;
				}
				break;
			case STATUS_CUSTOM_F5:
			case STATUS_CUSTOM_F6:
			case STATUS_CUSTOM_F7:
			case STATUS_CUSTOM_F8:
				if (customHandler) customHandler(size, event);
				break;
			default:
				break;
		}
		// add current byte to buffer
		waitForData--;
		event[size] = *byte;
		size++;
		if (waitForData == 0) { // got the whole event, need CRC
			waitForCRC = 1;
		}
	} else if (size==2) { // status byte
		if (*byte<0x80) { // third byte must be control
			decodeErr(PROTOCOL_ERR_NEED_CTRL);
			return 0;
		}
		switch (event[2]) {
			case STATUS_PIN_MODE:
			case STATUS_DIGITAL_PORT_REPORT:
			case STATUS_DIGITAL_PORT_SET:
			case STATUS_DIGITAL_PIN_REPORT:
			case STATUS_DIGITAL_PIN_SET:
			case STATUS_ANALOG_PIN_REPORT:
			case STATUS_ANALOG_PIN_SET:
			case STATUS_PROTOCOL_VERSION:
			case STATUS_PROTOCOL_ENCODING:
			case STATUS_INFO:
			case STATUS_SIGNAL:
			case STATUS_INTERRUPT:
			case STATUS_EMERGENCY_STOP:
			case STATUS_SYSTEM_PAUSE:
			case STATUS_SYSTEM_RESUME:
			case STATUS_SYSTEM_RESET:
				waitForData = 2; // two more bytes needed
				break;
			case STATUS_SYSEX_START:
				waitForData = 1; // more data needed
				break;
			case STATUS_CUSTOM_F5:
			case STATUS_CUSTOM_F6:
			case STATUS_CUSTOM_F7:
			case STATUS_CUSTOM_F8:
				if (customHandler) customHandler(size, event);
				break;
			default:
				decodeErr(PROTOCOL_ERR_EVENT_UNKNOWN);
				return 0;
				break;
		}
		event[0] = *byte;
		size = 1;
	} else if (size==1) {  // sequence id
		if (*byte>0x80) { // second byte must be data
			decodeErr(PROTOCOL_ERR_NEED_DATA);
			return 0;
		}
		event[size]=*byte;
		size++;
	} else { // new event start
		if (*byte == STATUS_EVENT_BEGIN) { // first byte must be 0xFF
			size = 0;
			event[size] = *byte;
			size++;
		} else {
			decodeErr(PROTOCOL_ERR_START);
			return 0;
		}
	}
	return 0;
}

uint8_t encodeEvent(uint8_t cmd, uint8_t argc, uint8_t *argv, uint8_t *event) {
	uint8_t count = 2; // sequence id size
	uint8_t datastart = count;
	uint8_t datasize = 2;
	switch (cmd) {
		case STATUS_PIN_MODE: // events with channel id and 2 bytes of data
		case STATUS_DIGITAL_PORT_REPORT:
		case STATUS_DIGITAL_PORT_SET:
		case STATUS_DIGITAL_PIN_REPORT:
		case STATUS_DIGITAL_PIN_SET:
		case STATUS_ANALOG_PIN_REPORT:
		case STATUS_ANALOG_PIN_SET:
			count += 4;
			event[0] = 255;
			event[1] = sequenceId;
			event[datastart] = (cmd & 0xF0) + (argc & 0x0F); // MSB=status + LSB=pin or port, bits are zeroed to prevent wrong input value
			if (argv) event[datastart+1] = BIT_CLEAR(argv[0], BIT(7));
			if (argv) event[datastart+2] = BIT_CLEAR(argv[1], BIT(7));
			break;
		case STATUS_PROTOCOL_VERSION: // events with 1-2 bytes of data (repeat che status byte in the 2nd in case of single data byte)
		case STATUS_PROTOCOL_ENCODING:
		case STATUS_INFO:
		case STATUS_SIGNAL:
		case STATUS_INTERRUPT:
		case STATUS_EMERGENCY_STOP:
		case STATUS_SYSTEM_PAUSE:
		case STATUS_SYSTEM_RESUME:
			count += 4;
			event[0] = 255;
			event[1] = sequenceId;
			event[datastart] = cmd;
			event[datastart+1] = BIT_CLEAR(argc, BIT(7));
			if (argv) event[datastart+2] = BIT_CLEAR(*argv, BIT(7)); else event[datastart+2] = cmd;
			break;
		case STATUS_SYSTEM_RESET: // events without any data
			count += 4;
			event[0] = 255;
			event[1] = sequenceId;
			event[datastart] = cmd;
			event[datastart+1] = cmd;
			event[datastart+2] = cmd;
			break;
		case STATUS_SYSEX_START: // sysex events 1+ bytes of data, realtime events, extended sysex, ...
			datasize += cbEvalEnc(argc-2);
			count += 1+datasize+2;
			event[0] = 255;
			event[1] = sequenceId;
			event[datastart] = cmd; // sysex start
			cbEncoder(argv, datasize, &event[datastart+1]);
			event[count-2] = STATUS_SYSEX_END;
			break;
		default:
			// TODO wrong event
			return 0;
			break;
	}
	event[count-1] = CRC8(count,event);
	sequenceId++;
	return count;
}

uint8_t encodePinMode(uint8_t *result, uint8_t pin, uint8_t mode) {
	return encodeEvent(STATUS_PIN_MODE, pin, &mode, result);
}

uint8_t encodeReportDigitalPort(uint8_t *result, uint8_t port) {
	return encodeEvent(STATUS_DIGITAL_PORT_REPORT, port, NULL, result);
}

uint8_t encodeSetDigitalPort(uint8_t *result, uint8_t port, uint8_t value) {
	uint8_t bytes[2];
	bytes[0] = MSB16(value);
	bytes[1] = LSB16(value);
	return encodeEvent(STATUS_DIGITAL_PORT_SET, port, bytes, result);
}

uint8_t encodeReportDigitalPin(uint8_t *result, uint8_t pin) {
	return encodeEvent(STATUS_DIGITAL_PIN_REPORT, pin, NULL, result);
}

uint8_t encodeSetDigitalPin(uint8_t *result, uint8_t pin, uint8_t value) {
	uint8_t bytes[2];
	bytes[0] = MSB16(value);
	bytes[1] = LSB16(value);
	return encodeEvent(STATUS_DIGITAL_PIN_SET, pin, bytes, result);
}

uint8_t encodeReportAnalogPin(uint8_t *result, uint8_t pin) {
	return encodeEvent(STATUS_ANALOG_PIN_REPORT, pin, NULL, result);
}

uint8_t encodeSetAnalogPin(uint8_t *result, uint8_t pin, uint8_t value) {
	uint8_t bytes[2];
	bytes[0] = MSB16(value);
	bytes[1] = LSB16(value);
	return encodeEvent(STATUS_ANALOG_PIN_SET, pin, bytes, result);
}

uint8_t encodeProtocolVersion(uint8_t *result) {
	uint8_t byte = PROTOCOL_VERSION_MINOR;
	return encodeEvent(STATUS_PROTOCOL_VERSION, PROTOCOL_VERSION_MAJOR, &byte, result);
}

uint8_t encodeProtocolEncoding(uint8_t *result, uint8_t proto) {
	return encodeEvent(STATUS_PROTOCOL_ENCODING, proto, NULL, result);
}

uint8_t encodeInfo(uint8_t *result, uint8_t info, uint8_t value) {
	if (value) return encodeEvent(STATUS_INFO, info, &value, result);
	else return encodeEvent(STATUS_INFO, info, NULL, result);
}

uint8_t encodeSignal(uint8_t *result, uint8_t key, uint8_t value) {
	return encodeEvent(STATUS_SIGNAL, key, &value, result);
}

uint8_t encodeInterrupt(uint8_t *result, uint8_t key, uint8_t value) {
	return encodeEvent(STATUS_INTERRUPT, key, &value, result);
}

uint8_t encodeEmergencyStop(uint8_t *result, uint8_t group) {
	return encodeEvent(STATUS_EMERGENCY_STOP, group, NULL, result);
}

uint8_t encodeSystemPause(uint8_t *result, uint16_t delay) {
	return encodeEvent(STATUS_SYSTEM_PAUSE, delay, NULL, result);
}

uint8_t encodeSystemResume(uint8_t *result, uint16_t time) {
	return encodeEvent(STATUS_SYSTEM_RESUME, time, NULL, result);
}

uint8_t encodeSystemReset(uint8_t *result, uint8_t mode) {
	return encodeEvent(STATUS_SYSTEM_RESET, mode, NULL, result);
}

uint8_t encodeSysex(uint8_t *result, uint8_t argc, uint8_t *argv) {
	return encodeEvent(STATUS_SYSEX_START, argc, argv, result);
}

uint8_t encodeTask(uint8_t *result, task_t *task, uint8_t error) {
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
	return encodeSysex(result, 3+tasklen, data);
}

