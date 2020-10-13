
#include <protocol/firmata.h>

// callback functions
cbf_varg printString = NULL;
cbf_varg printErr = NULL;
cbf_7bit_t cbEncoder = encode7bit;
cbf_7bit_t cbDecoder = decode7bit;
cbf_simple_t cbSimple = NULL;
cbf_sysex_t cbSysex = NULL;

// input message handling
static uint8_t waitForData; // this flag says the next serial input will be data
static uint8_t executeMultiByteCommand; // execute this after getting multi-byte data
static uint8_t multiByteChannel; // channel data for multiByteCommands
static uint8_t storedInputData[PROTOCOL_FIRMATA_MAX_DATA_PER_MSG]; // multi-byte data
static bool parsingSysex;
static uint8_t sysexBytesRead;

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

bool collect(uint8_t inputData) {
	uint8_t command;
	// if we are currently parsing a sysex ...
	if (parsingSysex) {
		if (inputData == CMD_END_SYSEX) {
			// stop sysex byte
			parsingSysex = false;
			// signal the completed message is in storedInputData
			return true;
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
			return true;
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
			case CMD_START_SYSEX:
				parsingSysex = true;
				storedInputData[0] = CMD_START_SYSEX;
				sysexBytesRead = 1;
				break;
			case CMD_DIGITAL_MESSAGE:
			case CMD_ANALOG_MESSAGE:
			case CMD_SET_PIN_MODE:
			case CMD_SET_DIGITAL_PIN_VALUE:
				waitForData = 2; // two data bytes needed
				storedInputData[0] = command;
				break;
			case CMD_REPORT_ANALOG:
			case CMD_REPORT_DIGITAL:
				waitForData = 1; // one data byte needed
				storedInputData[0] = command;
				break;
			case CMD_SYSTEM_RESET:
			case CMD_REPORT_VERSION:
				storedInputData[0] = command;
				// signal the completed message is in storedInputData
				return true;
				break;
			case CMD_EID_NON_REALTIME:
				// discard spurious realtime session ending
			default:
				// unknown starting bytes are silently discarded
				storedInputData[0] = 0;
				break;
		}
	}
	return false;
}

uint8_t measureMsg(void) {
	switch(storedInputData[0]) {
		case CMD_START_SYSEX:
			return sysexBytesRead;
			break;
		case CMD_DIGITAL_MESSAGE:
		case CMD_ANALOG_MESSAGE:
		case CMD_SET_PIN_MODE:
		case CMD_SET_DIGITAL_PIN_VALUE:
			return 3;
			break;
		case CMD_REPORT_ANALOG:
		case CMD_REPORT_DIGITAL:
			return 2;
			break;
		case CMD_SYSTEM_RESET:
		case CMD_REPORT_VERSION:
			return 1;
			break;
		default:
			return 0;
			break;
	}
}

void storeMsg(uint8_t *msg) {
	switch(msg[0]) {
		case CMD_START_SYSEX:
			for(int i=0;i<sysexBytesRead;i++) {
				msg[i] = storedInputData[i];
			}
			break;
		case CMD_DIGITAL_MESSAGE:
		case CMD_ANALOG_MESSAGE:
		case CMD_SET_PIN_MODE:
		case CMD_SET_DIGITAL_PIN_VALUE:
			msg[0] = storedInputData[0];
			msg[1] = storedInputData[1];
			msg[2] = storedInputData[2];
			break;
		case CMD_REPORT_ANALOG:
		case CMD_REPORT_DIGITAL:
			msg[0] = storedInputData[0];
			msg[1] = storedInputData[1];
			break;
		case CMD_SYSTEM_RESET:
		case CMD_REPORT_VERSION:
			msg[0] = storedInputData[0];
			break;
		default:
			msg[0] = 0;
			break;
	}
}

void dispatchMsg(uint8_t *msg) {
	if (msg == NULL) msg = storedInputData;
	switch(msg[0]) {
		case CMD_DIGITAL_MESSAGE:
		case CMD_ANALOG_MESSAGE:
			(*cbSimple)(msg[0], multiByteChannel, (msg[1] << 7) + msg[2]);
			break;
		case CMD_SET_PIN_MODE:
		case CMD_SET_DIGITAL_PIN_VALUE:
			(*cbSimple)(msg[0], msg[2], msg[1]);
			break;
		case CMD_REPORT_ANALOG:
		case CMD_REPORT_DIGITAL:
			(*cbSimple)(msg[0], multiByteChannel, msg[1]);
			break;
		case CMD_SYSTEM_RESET:
		case CMD_REPORT_VERSION:
			(*cbSimple)(msg[0], 0, 0);
			break;
		case CMD_START_SYSEX:
			switch (msg[1]) { // first byte is sysex start, second byte is command
				case CMD_EID_REPORT_FIRMWARE:
					(*cbSysex)(msg[1], 0, NULL);
					break;
				default:
					(*cbSysex)(msg[1], sysexBytesRead - 1, msg + 1);
					break;
			}
			break;
		default:
			break;
	}
}

void printMsg(uint8_t count, uint8_t *msg) {
	if (msg == NULL) {
		count = sysexBytesRead;
		msg = storedInputData;
	}
	switch(msg[0]) {
		case CMD_START_SYSEX:
			printString("%02x ", msg[1]);
			for(int i=1;i<count;i++) {
				printString("%02x ", msg[i]);
			}
			printString("(");
			switch(msg[1]) {
				case CMD_EID_EXTEND:
					break;
				case CMD_EID_USER_CHANGE_ENCODING:
					break;
				case CMD_EID_USER_REPORT_ENCODING:
					break;
				case CMD_EID_USER_SIGLAG:
					printString("SysEx CMD_EID_USER_SIGLAG %dms", msg[2]);
					break;
				case CMD_EID_USER_DELAYALL:
					printString("Not implemented");
					break;
				case CMD_EID_USER5:
					printString("Not implemented");
					break;
				case CMD_EID_USER6:
					printString("Not implemented");
					break;
				case CMD_EID_USER7:
					printString("Not implemented");
					break;
				case CMD_EID_USER8:
					printString("Not implemented");
					break;
				case CMD_EID_USER9:
					printString("Not implemented");
					break;
				case CMD_EID_USERA:
					printString("Not implemented");
					break;
				case CMD_EID_USERB:
					printString("Not implemented");
					break;
				case CMD_EID_USERC:
					printString("Not implemented");
					break;
				case CMD_EID_USERD:
					printString("Not implemented");
					break;
				case CMD_EID_USERE:
					printString("Not implemented");
					break;
				case CMD_EID_USERF:
					printString("Not implemented");
					break;
				case CMD_EID_RCOUT:
					printString("Not implemented");
					break;
				case CMD_EID_RCIN:
					printString("Not implemented");
					break;
				case CMD_EID_DEVICE_QUERY:
					printString("Not implemented");
					break;
				case CMD_EID_DEVICE_RESPONSE:
					printString("Not implemented");
					break;
				case CMD_EID_SERIAL_MESSAGE:
					printString("Not implemented");
					break;
				case CMD_EID_ENCODER_DATA:
					printString("Not implemented");
					break;
				case CMD_EID_ACCELSTEPPER_DATA:
					printString("Not implemented");
					break;
				case CMD_EID_REPORT_DIGITAL_PIN:
					printString("Not implemented");
					break;
				case CMD_EID_EXTENDED_REPORT_ANALOG:
					printString("Not implemented");
					break;
				case CMD_EID_REPORT_FEATURES:
					printString("Not implemented");
					break;
				case CMD_EID_SERIAL_DATA2:
					printString("Not implemented");
					break;
				case CMD_EID_SPI_DATA:
					printString("Not implemented");
					break;
				case CMD_EID_ANALOG_MAPPING_QUERY:
					printString("Not implemented");
					break;
				case CMD_EID_ANALOG_MAPPING_RESPONSE:
					printString("Not implemented");
					break;
				case CMD_EID_CAPABILITY_QUERY:
					printString("Not implemented");
					break;
				case CMD_EID_CAPABILITY_RESPONSE:
					printString("Not implemented");
					break;
				case CMD_EID_PIN_STATE_QUERY:
					printString("Not implemented");
					break;
				case CMD_EID_PIN_STATE_RESPONSE:
					printString("Not implemented");
					break;
				case CMD_EID_EXTENDED_ANALOG:
					printString("Not implemented");
					break;
				case CMD_EID_SERVO_CONFIG:
					printString("Not implemented");
					break;
				case CMD_EID_STRING_DATA:
					printString("Not implemented");
					break;
				case CMD_EID_STEPPER_DATA:
					printString("Not implemented");
					break;
				case CMD_EID_ONEWIRE_DATA:
					printString("Not implemented");
					break;
				case CMD_EID_SHIFT_DATA:
					printString("Not implemented");
					break;
				case CMD_EID_I2C_REQUEST:
					printString("Not implemented");
					break;
				case CMD_EID_I2C_REPLY:
					printString("Not implemented");
					break;
				case CMD_EID_I2C_CONFIG:
					printString("Not implemented");
					break;
				case CMD_EID_REPORT_FIRMWARE:
					printString("Not implemented");
					break;
				case CMD_EID_SAMPLING_INTERVAL:
					printString("Not implemented");
					break;
				case CMD_EID_SCHEDULER_DATA:
					printString("Not implemented");
					break;
				case CMD_EID_ANALOG_CONFIG:
					printString("Not implemented");
					break;
				case CMD_EID_NON_REALTIME:
					printString("Not implemented");
					break;
				case CMD_EID_REALTIME:
					printString("Not implemented");
					break;
				default:
					printString("Unknown SysEx type %#x", msg[1]);
					break;
			}
			printString(")");
			break;
		case CMD_DIGITAL_MESSAGE:
		case CMD_ANALOG_MESSAGE:
		case CMD_SET_PIN_MODE:
		case CMD_SET_DIGITAL_PIN_VALUE:
			printString("%02x %02x %02x\n", msg[0], msg[1], msg[2]);
			break;
		case CMD_REPORT_ANALOG:
		case CMD_REPORT_DIGITAL:
			printString("%02x %02x\n", msg[0], msg[1]);
			break;
		case CMD_SYSTEM_RESET:
		case CMD_REPORT_VERSION:
			printString("%02x\n", msg[0]);
			break;
		default:
			printString("NULL\n");
			break;
	}
}

