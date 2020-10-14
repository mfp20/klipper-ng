
#include <protocol/firmata.h>
#include <stdlib.h> // for malloc

// callback functions
cbf_varg_t printString = NULL;
cbf_varg_t printErr = NULL;
cbf_7bit_t cbEncoder = encode7bit;
cbf_7bit_t cbDecoder = decode7bit;

// input message handling
static uint8_t waitForData; // this flag says the next serial input will be data
static bool parsingSysex;
uint8_t multiByteChannel; // channel data for multiByteCommands
uint8_t storedInputData[PROTOCOL_MAX_DATA]; // multi-byte data
uint8_t sysexBytesRead;

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

uint8_t receiveMsg(uint8_t inputData) {
	uint8_t command;
	// if we are currently parsing a sysex ...
	if (parsingSysex) {
		if (inputData == CMD_SYSEX_END) {
			// stop sysex byte
			parsingSysex = false;
			// signal the completed message is in storedInputData
			return sysexBytesRead;
		} else {
			// normal data byte - add to buffer
			storedInputData[sysexBytesRead] = inputData;
			sysexBytesRead++;
		}
		// ... or we are waiting for missing bytes of a standard command ...
	} else if ( (waitForData > 0) && (inputData < 128) ) {
		waitForData--;
		storedInputData[waitForData+1] = inputData;
		if ( (waitForData == 0) && storedInputData[0] ) { // got the whole message
			// signal the completed message is in storedInputData
			return sysexBytesRead;
		}
		// ... or this is the first byte of a new command.
	} else {
		// remove channel info from command byte if less than 0xF0
		if (inputData < 0xF0) {
			command = inputData & 0xF0;
			multiByteChannel = inputData & 0x0F;
		} else {
			command = inputData;
			// commands in the 0xF* range don't use channel data
		}
		switch (command) {
			case CMD_SYSEX_START:
				parsingSysex = true;
				storedInputData[0] = CMD_SYSEX_START;
				sysexBytesRead = 1;
				break;
		case CMD_PIN_MODE:
		case CMD_DIGITAL_PORT_DATA:
		case CMD_DIGITAL_PIN_DATA:
		case CMD_ANALOG_PIN_DATA:
				waitForData = 2; // two data bytes needed
				storedInputData[0] = command;
				break;
		case CMD_DIGITAL_PORT_REPORT:
		case CMD_DIGITAL_PIN_REPORT:
		case CMD_ANALOG_PIN_REPORT:
				waitForData = 1; // one data byte needed
				storedInputData[0] = command;
				break;
			case CMD_SYSTEM_RESET:
			case CMD_VERSION_REPORT:
				storedInputData[0] = command;
				sysexBytesRead = 1;
				// signal the completed message is in storedInputData
				return sysexBytesRead;
				break;
			case SYSEX_REALTIME_END:
				// discard spurious realtime session ending
			default:
				// unknown starting bytes are silently discarded
				storedInputData[0] = 0;
				break;
		}
	}
	return 0;
}

uint8_t measureMsg(void) {
	switch(storedInputData[0]) {
		case CMD_SYSEX_START:
			return sysexBytesRead;
			break;
		case CMD_PIN_MODE:
		case CMD_DIGITAL_PORT_DATA:
		case CMD_DIGITAL_PIN_DATA:
		case CMD_ANALOG_PIN_DATA:
			return 3;
			break;
		case CMD_DIGITAL_PORT_REPORT:
		case CMD_DIGITAL_PIN_REPORT:
		case CMD_ANALOG_PIN_REPORT:
			return 2;
			break;
		case CMD_SYSTEM_RESET:
		case CMD_VERSION_REPORT:
			return 1;
			break;
		default:
			return 0;
			break;
	}
}

void storeMsg(uint8_t *msg) {
	switch(msg[0]) {
		case CMD_SYSEX_START:
			for(int i=0;i<sysexBytesRead;i++) {
				msg[i] = storedInputData[i];
			}
			break;
		case CMD_PIN_MODE:
		case CMD_DIGITAL_PORT_DATA:
		case CMD_DIGITAL_PIN_DATA:
		case CMD_ANALOG_PIN_DATA:
			msg[0] = storedInputData[0];
			msg[1] = storedInputData[1];
			msg[2] = storedInputData[2];
			break;
		case CMD_DIGITAL_PORT_REPORT:
		case CMD_DIGITAL_PIN_REPORT:
		case CMD_ANALOG_PIN_REPORT:
			msg[0] = storedInputData[0];
			msg[1] = storedInputData[1];
			break;
		case CMD_SYSTEM_RESET:
		case CMD_VERSION_REPORT:
			msg[0] = storedInputData[0];
			break;
		default:
			msg[0] = 0;
			break;
	}
}

void printMsg(uint8_t count, uint8_t *msg) {
	if (msg == NULL) {
		count = sysexBytesRead;
		msg = storedInputData;
	}
	switch(msg[0]) {
		case CMD_SYSEX_START:
			printString("%02x ", msg[1]);
			for(int i=1;i<count;i++) {
				printString("%02x ", msg[i]);
			}
			printString("(");
			switch(msg[1]) {
				case SYSEX_EXTEND:
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
				case SYSEX_REALTIME_END:
					printString("Not implemented");
					break;
				case SYSEX_REALTIME_START:
					printString("Not implemented");
					break;
				default:
					printString("Unknown SysEx type %#x", msg[1]);
					break;
			}
			printString(")");
			break;
		case CMD_PIN_MODE:
		case CMD_DIGITAL_PORT_DATA:
		case CMD_DIGITAL_PIN_DATA:
		case CMD_ANALOG_PIN_DATA:
			printString("%02x %02x %02x\n", msg[0], msg[1], msg[2]);
			break;
		case CMD_DIGITAL_PORT_REPORT:
		case CMD_DIGITAL_PIN_REPORT:
		case CMD_ANALOG_PIN_REPORT:
			printString("%02x %02x\n", msg[0], msg[1]);
			break;
		case CMD_SYSTEM_RESET:
		case CMD_VERSION_REPORT:
			printString("%02x\n", msg[0]);
			break;
		default:
			printString("NULL\n");
			break;
	}
}

