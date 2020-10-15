
#include <protocol.h>
#include <stdlib.h> // for malloc

uint8_t evalEncode(uint8_t count) {
	return 0;
}

uint8_t evalDecode(uint8_t count) {
	return 0;
}

void encode7bit(uint8_t *data, uint8_t count, uint8_t *result) {
	int i = 0;
	for (int j=0;j<count;j++) {
		result[i] = (data[j] & 0b01111111); // LSB, to use 0bXXXXXXXX notation enable -std=gnu++17 gcc extensions or similarly on clang
		result[i+1] = (data[j] >> 7 & 0b01111111); // MSB
		i = i+2;
	}
}

void decode7bit(uint8_t *data, uint8_t count, uint8_t *result) {
	uint8_t bufferLength = (count - 1) / 2;
	uint8_t i = 1;
	uint8_t j = 0;
	while (j < bufferLength) {
		result[j] = data[i];
		i++;
		result[j] += (data[i] << 7);
		i++;
		j++;
	}
}

uint8_t evalEncodeCompat(uint8_t count) {
	return 0;
}

uint8_t evalDecodeCompat(uint8_t count) {
	return 0;
}

void encode7bitCompat(uint8_t *data, uint8_t count, uint8_t *result) {
	uint8_t previous = 0;
	uint8_t shift = 0;
	int i = 0;
	for(int j=0;j<count;j++) {
		if (shift == 0) {
			result[i] = (data[j] & 0x7f);
			i++;
			shift++;
			previous = data[j] >> 7;
		} else {
			result[i] = (((data[j] << shift) & 0x7f) | previous);
			i++;
			if (shift == 6) {
				result[i] = (data[j] >> 1);
				i++;
				shift = 0;
			} else {
				shift++;
				previous = data[j] >> (8 - shift);
			}
		}
	}
	if (shift > 0) {
		i++;
		result[i] = (previous);
	}
}

void decode7bitCompat(uint8_t *data, uint8_t count, uint8_t *result) {
	for (int i = 0; i < count; i++) {
		int j = i << 3;
		int pos = j / 7;
		uint8_t shift = j % 7;
		result[i] = (data[pos] >> shift) | ((result[pos + 1] << (7 - shift)) & 0xFF);
	}
}

// callback functions
cbf_varg_t printString = NULL;
cbf_varg_t printErr = NULL;
cbf_eval_t cbEvalEnc = evalEncode;
cbf_coder_t cbEncoder = encode7bit;
cbf_eval_t cbEvalDec = evalDecode;
cbf_coder_t cbDecoder = decode7bit;

// message handling
static uint8_t waitForData = 0;
uint16_t deltaTime = 0;
uint8_t eventBuffer[PROTOCOL_MAX_DATA];
uint8_t eventSize = 0;

void resetBuffer(void) {}

uint8_t bufferCopy(uint8_t *event) {
	switch(event[0]) {
		case STATUS_SYSEX_START:
			for(int i=0;i<eventSize;i++) {
				event[i] = eventBuffer[i];
			}
			break;
		case STATUS_PIN_MODE:
		case STATUS_DIGITAL_PORT_DATA:
		case STATUS_DIGITAL_PIN_DATA:
		case STATUS_ANALOG_PIN_DATA:
			event[0] = eventBuffer[0];
			event[1] = eventBuffer[1];
			event[2] = eventBuffer[2];
			break;
		case STATUS_DIGITAL_PORT_REPORT:
		case STATUS_DIGITAL_PIN_REPORT:
		case STATUS_ANALOG_PIN_REPORT:
			event[0] = eventBuffer[0];
			event[1] = eventBuffer[1];
			break;
		case STATUS_SYSTEM_RESET:
		case STATUS_VERSION_REPORT:
			event[0] = eventBuffer[0];
			break;
		default:
			event[0] = 0;
			break;
	}
	return eventSize;
}

void printEvent(uint8_t count, uint8_t *event) {
	if (event == NULL) {
		count = eventSize;
		event = eventBuffer;
	}
	switch(event[0]) {
		case STATUS_SYSEX_START:
			printString("%02x ", event[1]);
			for(int i=1;i<count;i++) {
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
			printString("%02x %02x %02x\n", event[0], event[1], event[2]);
			break;
		case STATUS_DIGITAL_PORT_REPORT:
		case STATUS_DIGITAL_PIN_REPORT:
		case STATUS_ANALOG_PIN_REPORT:
			printString("%02x %02x\n", event[0], event[1]);
			break;
		case STATUS_SYSTEM_RESET:
		case STATUS_VERSION_REPORT:
			printString("%02x\n", event[0]);
			break;
		default:
			printString("NULL\n");
			break;
	}
}

uint8_t decodeEvent(uint8_t byte) {
	if(waitForData) { // need more data to complete the current event
		switch (eventBuffer[1+PROTOCOL_TICK_BYTES]) {
			case STATUS_SYSEX_START:
				if (byte == STATUS_SYSEX_END) {
					waitForData = 0;
					eventBuffer[eventSize] = byte;
					eventSize++;
					// signal the completed event
					return eventSize;
				} else {
					// MSb must be 0 because those must be data bytes (ie: byte value <= 127)
					if (byte>0x80) {
						// TODO signal transmit error
						// reset
						waitForData = 0;
						eventBuffer[0] = 0;
						eventSize = 0;
						return 0;
					}
					// normal data byte -> add to buffer
					eventBuffer[eventSize] = byte;
					eventSize++;
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
				if ((byte<0x80)||(byte!=eventBuffer[1+PROTOCOL_TICK_BYTES])) {
					// TODO signal transmit error
					// reset
					waitForData = 0;
					eventBuffer[0] = 0;
					eventSize = 0;
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
				if (byte>0x80) {
					// TODO signal transmit error
					// reset
					waitForData = 0;
					eventBuffer[0] = 0;
					eventSize = 0;
					return 0;
				}
				break;
			default:
				break;
		}
		waitForData--;
		eventBuffer[eventSize] = byte;
		eventSize++;
		if (waitForData == 0) { // got the whole event
			// signal the completed event
			return eventSize;
		}
	} else if (eventSize==1+PROTOCOL_TICK_BYTES) { // status bytes start
		switch (eventBuffer[1+PROTOCOL_TICK_BYTES]) {
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
				// reset
				eventBuffer[0] = 0;
				eventSize = 0;
				return 0;
				break;
		}
		eventBuffer[0] = byte;
		eventSize = 1;
	} else if (eventSize>0 && eventSize<1+PROTOCOL_TICK_BYTES) {  // delta time bytes
		if (byte>0x80) {
			// TODO signal error
			// reset
			waitForData = 0;
			eventBuffer[0] = 0;
			eventSize = 0;
			return 0;
		}
		eventBuffer[eventSize]=byte;
		eventSize++;
	} else { // new event start
		if (byte == 255) {
			eventSize = 0;
			eventBuffer[eventSize] = byte;
			eventSize++;
		}
	}
	return 0;
}

uint8_t encodeEvent(uint8_t cmd, uint8_t argc, uint8_t *argv, uint8_t *event) {
	uint8_t count = 1+PROTOCOL_TICK_BYTES;
	uint8_t start = count;
	uint8_t datasize;
	switch (cmd) {
		case STATUS_ENCODING_SWITCH:
			break;
			// events without data repeat the command to reduce the chance of transmit error
		case STATUS_INTERRUPT:
		case STATUS_EMERGENCY_STOP1:
		case STATUS_EMERGENCY_STOP2:
		case STATUS_EMERGENCY_STOP3:
		case STATUS_EMERGENCY_STOP4:
		case STATUS_SYSTEM_HALT:
		case STATUS_SYSTEM_RESET:
			count += 3;
			event = (uint8_t*)malloc(sizeof(uint8_t)*count);
			event[0] = 255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			event[start] = cmd;
			event[start+1] = cmd;
			event[start+2] = cmd;
			return count;
			break;
		case STATUS_MICROSTAMP_REPORT:
		case STATUS_TIMESTAMP_REPORT:
		case STATUS_CONGESTION_REPORT:
		case STATUS_VERSION_REPORT:
		case STATUS_PIN_MODE:
		case STATUS_SYSTEM_PAUSE:
		case STATUS_SYSTEM_RESUME:
			count += 3;
			event = (uint8_t*)malloc(sizeof(uint8_t)*count);
			event[0] = 255;
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
			event = (uint8_t*)malloc(sizeof(uint8_t)*count);
			event[0] = 255;
			event[1] = (uint8_t)(deltaTime & 0x00FF); // LSB
			event[2] = (uint8_t)(deltaTime & 0xFF00); // MSB
			// MSB=status + LSB=pin or port, bits are zeroed to prevent wrong input value
			event[start] = (cmd & 0b11110000) +
				(argc & 0b00001111);
			event[start+1] = argv[0];
			event[start+2] = argv[1];
			return count;
			break;
		case STATUS_SYSEX_START:
			switch(argv[0]) {
				case SYSEX_MOD_EXTEND:
					count += 4;
					event = (uint8_t*)malloc(sizeof(uint8_t)*count);
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
					event = (uint8_t*)malloc(sizeof(uint8_t)*count);
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
					event = (uint8_t*)malloc(sizeof(uint8_t)*count);
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

