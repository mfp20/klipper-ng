
#include <protocol/firmata.h>

// callback functions
cbf_string_t cbString = NULL;
cbf_simple_t cbSimple = NULL;
cbf_sysex_t cbSysex = NULL;
cbf_7bit_t cbEncoder = encode7bit;
cbf_7bit_t cbDecoder = decode7bit;

// input message handling
static uint8_t waitForData; // this flag says the next serial input will be data
static uint8_t executeMultiByteCommand; // execute this after getting multi-byte data
static uint8_t multiByteChannel; // channel data for multiByteCommands
static uint8_t storedInputData[PROTOCOL_FIRMATA_MAX_DATA_PER_MSG]; // multi-byte data
static bool parsingSysex;
static uint8_t sysexBytesRead;

void collect(uint8_t inputData) {
	uint8_t command;
	// if we are currently parsing a sysex ...
	if (parsingSysex) {
		if (inputData == CMD_END_SYSEX) {
			//stop sysex byte
			parsingSysex = false;
			// fire off handler function
			if (cbSysex)
				switch (storedInputData[0]) { // first byte in buffer is command
					case CMD_EID_REPORT_FIRMWARE:
						(*cbSysex)(storedInputData[0], 0, NULL);
						break;
					default:
						(*cbSysex)(storedInputData[0], sysexBytesRead - 1, storedInputData + 1);
						break;
				}
		} else {
			// normal data byte - add to buffer
			storedInputData[sysexBytesRead] = inputData;
			sysexBytesRead++;
		}
		// ... or we are waiting for missing bytes of a standard command ...
	} else if ( (waitForData > 0) && (inputData < 128) ) {
		waitForData--;
		storedInputData[waitForData] = inputData;
		if ( (waitForData == 0) && executeMultiByteCommand ) { // got the whole message
			switch (executeMultiByteCommand) {
				case CMD_DIGITAL_MESSAGE:
				case CMD_ANALOG_MESSAGE:
					(*cbSimple)(executeMultiByteCommand, multiByteChannel, (storedInputData[0] << 7) + storedInputData[1]);
					break;
				case CMD_SET_PIN_MODE:
				case CMD_SET_DIGITAL_PIN_VALUE:
					(*cbSimple)(executeMultiByteCommand, storedInputData[1], storedInputData[0]);
					break;
				case CMD_REPORT_ANALOG:
				case CMD_REPORT_DIGITAL:
					(*cbSimple)(executeMultiByteCommand, multiByteChannel, storedInputData[0]);
					break;
			}
			executeMultiByteCommand = 0;
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
			case CMD_DIGITAL_MESSAGE:
			case CMD_ANALOG_MESSAGE:
			case CMD_SET_PIN_MODE:
			case CMD_SET_DIGITAL_PIN_VALUE:
				waitForData = 2; // two data bytes needed
				executeMultiByteCommand = command;
				break;
			case CMD_REPORT_ANALOG:
			case CMD_REPORT_DIGITAL:
				waitForData = 1; // one data byte needed
				executeMultiByteCommand = command;
				break;
			case CMD_START_SYSEX:
				parsingSysex = true;
				sysexBytesRead = 0;
				break;
			case CMD_SYSTEM_RESET:
			case CMD_REPORT_VERSION:
				(*cbSimple)(command, 0, 0);
				break;
		}
	}
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

