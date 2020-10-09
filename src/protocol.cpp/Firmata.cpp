
#include <protocol/Firmata.hpp>

/**
 * The Firmata class.
 * An instance named "Firmata" is created automatically for the user.
 */
Firmata::Firmata() {
	firmwareVersionCount = 0;
	firmwareVersionVector = 0;
	blinkVersionDisabled = false;
	systemReset();
}

/**
 * Initialize the default Serial transport at the default baud of 57600.
 */
void Firmata::begin(void) {
	begin(DEFAULT_BAUD);
}

/**
 * Initialize the default Serial transport and override the default baud.
 * Sends the protocol version to the host application followed by the firmware version and name.
 * blinkVersion is also called. To skip the call to blinkVersion, call Firmata.disableBlinkVersion()
 * before calling Firmata.begin(baud).
 * @param speed The baud to use. 57600 baud is the default value.
 */
void Firmata::begin(long speed) {
	hal.binConsole->begin(speed);
	blinkVersion();
	printVersion();         // send the protocol version
	printFirmwareVersion(); // send the firmware name and version
}

/**
 * Reassign the Firmata stream transport.
 * @param s A reference to the Stream transport object. This can be any type of
 * transport that implements the Stream interface. Some examples include Ethernet, WiFi
 * and other UARTs on the board (Serial1, Serial2, etc).
 */
void Firmata::begin(SerialStream &s) {
	hal.binConsole = &s;
	// do not call blinkVersion() here because some hardware such as the
	// Ethernet shield use pin 13
	printVersion();         // send the protocol version
	printFirmwareVersion(); // send the firmware name and version
}

/**
 * Send the Firmata protocol version to the Firmata host application.
 */
void Firmata::printVersion(void) {
	hal.binConsole->write(REPORT_VERSION);
	hal.binConsole->write(FIRMATA_PROTOCOL_MAJOR_VERSION);
	hal.binConsole->write(FIRMATA_PROTOCOL_MINOR_VERSION);
}

/**
 * Blink the Firmata protocol version to the onboard LEDs (if the board has an onboard LED).
 * If VERSION_BLINK_PIN is not defined in Boards.h for a particular board, then this method
 * does nothing.
 * The first series of flashes indicates the firmware major version (2 flashes = 2).
 * The second series of flashes indicates the firmware minor version (5 flashes = 5).
 */
void Firmata::blinkVersion(void) {
#if defined(VERSION_BLINK_PIN)
	if (blinkVersionDisabled) return;
	// flash the pin with the protocol version
	pin_mode(VERSION_BLINK_PIN, 1);
	strobeBlinkPin(VERSION_BLINK_PIN, FIRMATA_FIRMWARE_MAJOR_VERSION, 40, 210);
	usleep(250000);
	strobeBlinkPin(VERSION_BLINK_PIN, FIRMATA_FIRMWARE_MINOR_VERSION, 40, 210);
	usleep(125000);
#endif
}

/**
 * Provides a means to disable the version blink sequence on the onboard LED, trimming startup
 * time by a couple of seconds.
 * Call this before Firmata.begin(). It only applies when using the default Serial transport.
 */
void Firmata::disableBlinkVersion() {
	blinkVersionDisabled = true;
}

/**
 * Sends the firmware name and version to the Firmata host application. The major and minor version
 * numbers are the first 2 bytes in the message. The following bytes are the characters of the
 * firmware name.
 */
void Firmata::printFirmwareVersion(void) {
	uint8_t i;
	if (firmwareVersionCount) { // make sure that the name has been set before reporting
		startSysex();
		hal.binConsole->write(REPORT_FIRMWARE);
		hal.binConsole->write(firmwareVersionVector[0]); // major version number
		hal.binConsole->write(firmwareVersionVector[1]); // minor version number
		for (i = 2; i < firmwareVersionCount; ++i) {
			sendValueAsTwo7bitBytes(firmwareVersionVector[i]);
		}
		endSysex();
	}
}

void Firmata::setFirmwareVersion(uint8_t major, uint8_t minor) {
	firmata.setFirmwareNameAndVersion("src/Libknp.cpp", major, minor);
}

/**
 * Sets the name and version of the firmware. This is not the same version as the Firmata protocol
 * (although at times the firmware version and protocol version may be the same number).
 * @param name A pointer to the name char array
 * @param major The major version number
 * @param minor The minor version number
 */
void Firmata::setFirmwareNameAndVersion(const char *name, uint8_t major, uint8_t minor) {
	const char *firmwareName;
	const char *extension;

	// parse out ".cpp" and "applet/" that comes from using __FILE__
	extension = strstr(name, ".cpp");
	firmwareName = strrchr(name, '/');

	if (!firmwareName) {
		// windows
		firmwareName = strrchr(name, '\\');
	}
	if (!firmwareName) {
		// user passed firmware name
		firmwareName = name;
	} else {
		firmwareName ++;
	}

	if (!extension) {
		firmwareVersionCount = strlen(firmwareName) + 2;
	} else {
		firmwareVersionCount = extension - firmwareName + 2;
	}

	// in case anyone calls setFirmwareNameAndVersion more than once
	free(firmwareVersionVector);

	firmwareVersionVector = (uint8_t *) malloc(firmwareVersionCount + 1);
	firmwareVersionVector[firmwareVersionCount] = 0;
	firmwareVersionVector[0] = major;
	firmwareVersionVector[1] = minor;
	strncpy((char *)firmwareVersionVector + 2, firmwareName, firmwareVersionCount - 2);
}

/**
 * A wrapper for Stream::available()
 * @return The number of bytes remaining in the input stream buffer.
 */
int Firmata::available(void) {
	return hal.binConsole->available();
}

/**
 * Process incoming sysex messages. Handles REPORT_FIRMWARE and STRING_DATA internally.
 * Calls callback function for STRING_DATA and all other sysex messages.
 * @private
 */
void Firmata::processSysexMessage(void) {
	switch (storedInputData[0]) { //first byte in buffer is command
		case REPORT_FIRMWARE:
			printFirmwareVersion();
			break;
		case STRING_DATA:
			if (currentStringCallback) {
				uint8_t bufferLength = (sysexBytesRead - 1) / 2;
				uint8_t i = 1;
				uint8_t j = 0;
				while (j < bufferLength) {
					// The string length will only be at most half the size of the
					// stored input buffer so we can decode the string within the buffer.
					storedInputData[j] = storedInputData[i];
					i++;
					storedInputData[j] += (storedInputData[i] << 7);
					i++;
					j++;
				}
				// Make sure string is null terminated. This may be the case for data
				// coming from client libraries in languages that don't null terminate
				// strings.
				if (storedInputData[j - 1] != '\0') {
					storedInputData[j] = '\0';
				}
				(*currentStringCallback)((char *)&storedInputData[0]);
			}
			break;
		default:
			if (currentSysexCallback)
				(*currentSysexCallback)(storedInputData[0], sysexBytesRead - 1, storedInputData + 1);
	}
}


/**
 * Read a single int from the input stream. If the value is not = -1, pass it on to parse(byte)
 */
void Firmata::processInput(void) {
	int inputData = hal.binConsole->read(); // this is 'int' to handle -1 when no data
	if (inputData != -1) {
		parse(inputData);
	}
}

/**
 * Parse data from the input stream.
 * @param inputData A single byte to be added to the parser.
 */
void Firmata::parse(uint8_t inputData) {
	int command;

	// TODO make sure it handles -1 properly

	if (parsingSysex) {
		if (inputData == END_SYSEX) {
			//stop sysex byte
			parsingSysex = false;
			//fire off handler function
			processSysexMessage();
		} else {
			//normal data byte - add to buffer
			storedInputData[sysexBytesRead] = inputData;
			sysexBytesRead++;
		}
	} else if ( (waitForData > 0) && (inputData < 128) ) {
		waitForData--;
		storedInputData[waitForData] = inputData;
		if ( (waitForData == 0) && executeMultiByteCommand ) { // got the whole message
			switch (executeMultiByteCommand) {
				case ANALOG_MESSAGE:
					if (currentAnalogCallback) {
						(*currentAnalogCallback)(multiByteChannel,
								(storedInputData[0] << 7)
								+ storedInputData[1]);
					}
					break;
				case DIGITAL_MESSAGE:
					if (currentDigitalCallback) {
						(*currentDigitalCallback)(multiByteChannel,
								(storedInputData[0] << 7)
								+ storedInputData[1]);
					}
					break;
				case SET_PIN_MODE:
					setPinMode(storedInputData[1], storedInputData[0]);
					break;
				case SET_DIGITAL_PIN_VALUE:
					if (currentPinValueCallback)
						(*currentPinValueCallback)(storedInputData[1], storedInputData[0]);
					break;
				case REPORT_ANALOG:
					if (currentReportAnalogCallback)
						(*currentReportAnalogCallback)(multiByteChannel, storedInputData[0]);
					break;
				case REPORT_DIGITAL:
					if (currentReportDigitalCallback)
						(*currentReportDigitalCallback)(multiByteChannel, storedInputData[0]);
					break;
			}
			executeMultiByteCommand = 0;
		}
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
			case ANALOG_MESSAGE:
			case DIGITAL_MESSAGE:
			case SET_PIN_MODE:
			case SET_DIGITAL_PIN_VALUE:
				waitForData = 2; // two data bytes needed
				executeMultiByteCommand = command;
				break;
			case REPORT_ANALOG:
			case REPORT_DIGITAL:
				waitForData = 1; // one data byte needed
				executeMultiByteCommand = command;
				break;
			case START_SYSEX:
				parsingSysex = true;
				sysexBytesRead = 0;
				break;
			case SYSTEM_RESET:
				systemReset();
				break;
			case REPORT_VERSION:
				firmata.printVersion();
				break;
		}
	}
}

/**
 * @return Returns true if the parser is actively parsing data.
 */
bool Firmata::isParsingMessage(void) {
	return (waitForData > 0 || parsingSysex);
}

/**
 * @return Returns true if the SYSTEM_RESET message is being executed
 */
bool Firmata::isResetting(void) {
	return resetting;
}

/**
 * Send an analog message to the Firmata host application. The range of pins is limited to [0..15]
 * when using the ANALOG_MESSAGE. The maximum value of the ANALOG_MESSAGE is limited to 14 bits
 * (16384). To increase the pin range or value, see the documentation for the EXTENDED_ANALOG
 * message.
 * @param pin The analog pin to send the value of (limited to pins 0 - 15).
 * @param value The value of the analog pin (0 - 1024 for 10-bit analog, 0 - 4096 for 12-bit, etc).
 * The maximum value is 14-bits (16384).
 */
void Firmata::sendAnalog(uint8_t pin, int value) {
	// pin can only be 0-15, so chop higher bits
	hal.binConsole->write(ANALOG_MESSAGE | (pin & 0xF));
	sendValueAsTwo7bitBytes(value);
}

/* (intentionally left out asterix here)
 * STUB - NOT IMPLEMENTED
 * Send a single digital pin value to the Firmata host application.
 * @param pin The digital pin to send the value of.
 * @param value The value of the pin.
 */
void Firmata::sendDigital(uint8_t pin, int value) {
	/* TODO add single pin digital messages to the protocol, this needs to
	 * track the last digital data sent so that it can be sure to change just
	 * one bit in the packet.  This is complicated by the fact that the
	 * numbering of the pins will probably differ on Arduino, Wiring, and
	 * other boards.  The DIGITAL_MESSAGE sends 14 bits at a time, but it is
	 * probably easier to send 8 bit ports for any board with more than 14
	 * digital pins.
	 */

	// TODO: the digital message should not be sent on the serial port every
	// time sendDigital() is called.  Instead, it should add it to an int
	// which will be sent on a schedule.  If a pin changes more than once
	// before the digital message is sent on the serial port, it should send a
	// digital message for each change.

	//    if(value == 0)
	//        sendDigitalPortPair();
	pin=pin;
	value=value;
}

/**
 * Send an 8-bit port in a single digital message (protocol v2 and later).
 * Send 14-bits in a single digital message (protocol v1).
 * @param portNumber The port number to send. Note that this is not the same as a "port" on the
 * physical microcontroller. Ports are defined in order per every 8 pins in ascending order
 * of the Arduino digital pin numbering scheme. Port 0 = pins D0 - D7, port 1 = pins D8 - D15, etc.
 * @param portData The value of the port. The value of each pin in the port is represented by a bit.
 */
void Firmata::sendDigitalPort(uint8_t portNumber, int portData) {
	hal.binConsole->write(DIGITAL_MESSAGE | (portNumber & 0xF));
	hal.binConsole->write((uint8_t)portData % 128); // Tx bits 0-6
	hal.binConsole->write(portData >> 7);  // Tx bits 7-13
}

/**
 * Send a sysex message where all values after the command byte are packet as 2 7-bit bytes
 * (this is not always the case so this function is not always used to send sysex messages).
 * @param command The sysex command byte.
 * @param bytec The number of data bytes in the message (excludes start, command and end bytes).
 * @param bytev A pointer to the array of data bytes to send in the message.
 */
void Firmata::sendSysex(uint8_t command, uint8_t bytec, uint8_t *bytev) {
	uint8_t i;
	startSysex();
	hal.binConsole->write(command);
	for (i = 0; i < bytec; i++) {
		sendValueAsTwo7bitBytes(bytev[i]);
	}
	endSysex();
}

/**
 * Send a string to the Firmata host application.
 * @param command Must be STRING_DATA
 * @param string A pointer to the char string
 */
void Firmata::sendString(uint8_t command, const char *string) {
	sendSysex(command, strlen(string), (uint8_t *)string);
}

/**
 * Send a string to the Firmata host application.
 * @param string A pointer to the char string
 */
void Firmata::sendString(const char *string) {
	sendString(STRING_DATA, string);
}

/**
 * A wrapper for Stream::available().
 * Write a single byte to the output stream.
 * @param c The byte to be written.
 */
size_t Firmata::write(uint8_t c) {
	return hal.binConsole->write(c);
}

/**
 * Attach a generic sysex callback function to a command (options are: ANALOG_MESSAGE,
 * DIGITAL_MESSAGE, REPORT_ANALOG, REPORT DIGITAL, SET_PIN_MODE and SET_DIGITAL_PIN_VALUE).
 * @param command The ID of the command to attach a callback function to.
 * @param newFunction A reference to the callback function to attach.
 */
void Firmata::attach(uint8_t command, callbackFunction newFunction) {
	switch (command) {
		case ANALOG_MESSAGE: currentAnalogCallback = newFunction; break;
		case DIGITAL_MESSAGE: currentDigitalCallback = newFunction; break;
		case REPORT_ANALOG: currentReportAnalogCallback = newFunction; break;
		case REPORT_DIGITAL: currentReportDigitalCallback = newFunction; break;
		case SET_PIN_MODE: currentPinModeCallback = newFunction; break;
		case SET_DIGITAL_PIN_VALUE: currentPinValueCallback = newFunction; break;
	}
}

/**
 * Attach a callback function for the SYSTEM_RESET command.
 * @param command Must be set to SYSTEM_RESET or it will be ignored.
 * @param newFunction A reference to the system reset callback function to attach.
 */
void Firmata::attach(uint8_t command, systemResetCallbackFunction newFunction) {
	switch (command) {
		case SYSTEM_RESET: currentSystemResetCallback = newFunction; break;
	}
}

/**
 * Attach a callback function for the STRING_DATA command.
 * @param command Must be set to STRING_DATA or it will be ignored.
 * @param newFunction A reference to the string callback function to attach.
 */
void Firmata::attach(uint8_t command, stringCallbackFunction newFunction) {
	switch (command) {
		case STRING_DATA: currentStringCallback = newFunction; break;
	}
}

/**
 * Attach a generic sysex callback function to sysex command.
 * @param command The ID of the command to attach a callback function to.
 * @param newFunction A reference to the sysex callback function to attach.
 */
void Firmata::attach(uint8_t command, sysexCallbackFunction newFunction) {
	command=command;
	currentSysexCallback = newFunction;
}

/**
 * Detach a callback function for a specified command (such as SYSTEM_RESET, STRING_DATA,
 * ANALOG_MESSAGE, DIGITAL_MESSAGE, etc).
 * @param command The ID of the command to detatch the callback function from.
 */
void Firmata::detach(uint8_t command) {
	switch (command) {
		case SYSTEM_RESET: currentSystemResetCallback = NULL; break;
		case STRING_DATA: currentStringCallback = NULL; break;
		case START_SYSEX: currentSysexCallback = NULL; break;
		default:
						  attach(command, (callbackFunction)NULL);
	}
}

/**
 * Detach a callback function for a delayed task when using FirmataScheduler
 * @see FirmataScheduler
 * @param newFunction A reference to the delay task callback function to attach.
 */
void Firmata::attachDelayTask(delayTaskCallbackFunction newFunction) {
	delayTaskCallback = newFunction;
}

/**
 * Call the delayTask callback function when using FirmataScheduler. Must first attach a callback
 * using attachDelayTask.
 * @see FirmataScheduler
 * @param delay The amount of time to delay in milliseconds.
 */
void Firmata::delayTask(long delay) {
	if (delayTaskCallback) {
		(*delayTaskCallback)(delay);
	}
}

/**
 * @param pin The pin to get the configuration of.
 * @return The configuration of the specified pin.
 */
uint8_t Firmata::getPinMode(uint8_t pin) {
	return pinConfig[pin];
}

/**
 * Set the pin mode/configuration. The pin configuration (or mode) in Firmata represents the
 * current function of the pin. Examples are digital input or output, analog input, pwm, i2c,
 * serial (uart), etc.
 * @param pin The pin to configure.
 * @param config The configuration value for the specified pin.
 */
void Firmata::setPinMode(uint8_t pin, uint8_t config) {
	if (pinConfig[pin] == PIN_MODE_IGNORE)
		return;
	pinState[pin] = 0;
	pinConfig[pin] = config;
	if (currentPinModeCallback)
		(*currentPinModeCallback)(pin, config);
}

/**
 * @param pin The pin to get the state of.
 * @return The state of the specified pin.
 */
int Firmata::getPinState(uint8_t pin) {
	return pinState[pin];
}

/**
 * Set the pin state. The pin state of an output pin is the pin value. The state of an
 * input pin is 0, unless the pin has it's internal pull up resistor enabled, then the value is 1.
 * @param pin The pin to set the state of
 * @param state Set the state of the specified pin
 */
void Firmata::setPinState(uint8_t pin, int state) {
	pinState[pin] = state;
}

/**
 * Split a 16-bit byte into two 7-bit values and write each value.
 * @param value The 16-bit value to be split and written separately.
 */
void Firmata::sendValueAsTwo7bitBytes(int value) {
	hal.binConsole->write(value & 0b01111111); // LSB, to use 0bXXXXXXXX notation enable -std=gnu++17 gcc extensions or similarly on clang
	hal.binConsole->write(value >> 7 & 0b01111111); // MSB
}

/**
 * A helper method to write the beginning of a Sysex message transmission.
 */
void Firmata::startSysex(void) {
	hal.binConsole->write(START_SYSEX);
}

/**
 * A helper method to write the end of a Sysex message transmission.
 */
void Firmata::endSysex(void) {
	hal.binConsole->write(END_SYSEX);
}


// sysex callbacks
/*
 * this is too complicated for analogReceive, but maybe for Sysex?
 void Firmata::attachSysex(sysexFunction newFunction) {
 uint8_t i;
 uint8_t tmpCount = analogReceiveFunctionCount;
 analogReceiveFunction* tmpArray = analogReceiveFunctionArray;
 analogReceiveFunctionCount++;
 analogReceiveFunctionArray = (analogReceiveFunction*) calloc(analogReceiveFunctionCount, sizeof(analogReceiveFunction));
 for(i = 0; i < tmpCount; i++) {
 analogReceiveFunctionArray[i] = tmpArray[i];
 }
 analogReceiveFunctionArray[tmpCount] = newFunction;
 free(tmpArray);
 }
 */

/**
 * Resets the system state upon a SYSTEM_RESET message from the host software.
 * @private
 */
void Firmata::systemReset(void) {
	resetting = true;
	uint8_t i;

	waitForData = 0; // this flag says the next serial input will be data
	executeMultiByteCommand = 0; // execute this after getting multi-byte data
	multiByteChannel = 0; // channel data for multiByteCommands

	for (i = 0; i < MAX_DATA_BYTES; i++) {
		storedInputData[i] = 0;
	}

	parsingSysex = false;
	sysexBytesRead = 0;

	if (currentSystemResetCallback)
		(*currentSystemResetCallback)();

	resetting = false;
}

/**
 * Flashing the pin for the version number
 * @private
 * @param pin The pin the LED is attached to.
 * @param count The number of times to flash the LED.
 * @param onInterval The number of milliseconds for the LED to be ON during each interval.
 * @param offInterval The number of milliseconds for the LED to be OFF during each interval.
 */
void Firmata::strobeBlinkPin(uint8_t pin, int count, int onInterval, int offInterval) {
	uint8_t i;
	for (i = 0; i < count; i++) {
		usleep(offInterval*1000);
		pin_write(pin, 1);
		usleep(onInterval*1000);
		pin_write(pin, 0);
	}
}

