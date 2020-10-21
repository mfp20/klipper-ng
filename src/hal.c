
#include <hal.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

pin_status_t *pin_status = NULL;
uint8_t pin_status_size = 0;
pin_status_t *preferred_pin[16];
pin_status_t *pin_group[4][16];

void halInit(void) {
	arch_init();
	board_init();
	// init pin status and register console pins
	uint8_t last;
	switch(port[0].type) {
		case COMMPORT_TYPE_1WIRE:
			pin_status = (pin_status_t *)calloc(last+1, sizeof(pin_status_t));
			pin_status_size = pins_1wire[0];
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[pins_1wire[0]].mode = PIN_MODE_1WIRE;
			break;
		case COMMPORT_TYPE_TTY:
			last = MAX(pins_tty[0][0], pins_tty[0][1]);
			pin_status = (pin_status_t *)calloc(last+1, sizeof(pin_status_t));
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[pins_tty[0][0]].mode = PIN_MODE_TTY;
			pin_status[pins_tty[0][1]].mode = PIN_MODE_TTY;
			break;
		case COMMPORT_TYPE_I2C:
			last = MAX(pins_i2c[0][0], pins_i2c[0][1]);
			pin_status = (pin_status_t *)calloc(last+1, sizeof(pin_status_t));
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[pins_i2c[0][0]].mode = PIN_MODE_I2C;
			pin_status[pins_i2c[0][1]].mode = PIN_MODE_I2C;
			break;
		case COMMPORT_TYPE_SPI:
			last = MAX(pins_spi[0][0], pins_spi[0][1]);
			last = MAX(last, pins_spi[0][2]);
			last = MAX(last, pins_spi[0][3]);
			pin_status = (pin_status_t *)calloc(last+1, sizeof(pin_status_t));
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[pins_spi[0][0]].mode = PIN_MODE_SPI;
			pin_status[pins_spi[0][1]].mode = PIN_MODE_SPI;
			pin_status[pins_spi[0][2]].mode = PIN_MODE_SPI;
			pin_status[pins_spi[0][3]].mode = PIN_MODE_SPI;
			break;
	}

	// enable binConsole
	binConsole = &port[0];
}

void halRun(void) {
	arch_run();
	board_run();
}

uint16_t halPause(uint16_t delay) {
	// TODO pause system and report paused clock
	return millis();
}

uint16_t halResume(uint16_t time) {
	// TODO resume system and report starting clock
	return millis();
}

void halReset(uint8_t mode) {
	for (uint8_t i = 0; i < PIN_TOTAL; i++) {
		pinFree(i);
	}
}

uint8_t pinIsFree(uint8_t pin) {
	return pin_status[pin].mode;
}

void pinUse(uint8_t pin, uint8_t mode) {
	if (pin > pin_status_size) {
		pin_status = (pin_status_t *)realloc(pin_status, sizeof(pin_status_t)*pin);
		for(uint8_t i=pin_status_size;i<=pin;i++) {
			pin_status[i].mode = 0;
			pin_status[i].value = 0;
			pin_status[i].vextend = 0;
		}
		pin_status_size = pin;
	}
	pin_status[pin].mode = mode;
}

void pinPrefer(uint8_t pin, uint8_t pos) {
	if(pos!=0) preferred_pin[pos] = &pin_status[pin];
	else preferred_pin[pos] = NULL;
}

void pinGroup(uint8_t pin, uint8_t group, uint8_t pos) {
	if(pos!=0) pin_group[group][pos] = &pin_status[pin];
	else pin_group[group][pos] = NULL;
}

void pinMode(uint8_t pin, uint8_t mode) {
	pin_mode(pin,mode);
	pin_status[pin].mode = mode;
	switch (mode) {
		case PIN_MODE_PULLUP:
			break;
		case PIN_MODE_INPUT:
			break;
		case PIN_MODE_OUTPUT:
			break;
		case PIN_MODE_PWM:
			break;
		case PIN_MODE_ANALOG:
			break;
		case PIN_MODE_1WIRE:
			break;
		case PIN_MODE_TTY:
			break;
		case PIN_MODE_I2C:
			break;
		case PIN_MODE_SPI:
			break;
		case PIN_MODE_DEBOUNCE:
			break;
		case PIN_MODE_SHIFT:
			break;
		case PIN_MODE_STEPPER:
			break;
		case PIN_MODE_SERVO:
			break;
		case PIN_MODE_IGNORE:
			pin_status[pin].value = 0;
			pin_status[pin].vextend = 0;
			break;
		default:
			break;
	}
}

uint8_t pinRead(uint8_t pin, uint8_t timeout) {
	pin_status[pin].value = pin_read(pin, timeout);
	return pin_status[pin].value;
}

void pinWrite(uint8_t pin, uint8_t value) {
	pin_write(pin, value);
	pin_status[pin].value = value;
}

void pinFree(uint8_t pin) {
	// set pins with analog capability to analog input
	// and set digital pins to digital output
	if (PIN_IS_ANALOG(pin)) {
		pin_mode(pin,PIN_MODE_ANALOG);
	} else if (PIN_IS_DIGITAL(pin)) {
		pin_mode(pin,PIN_MODE_OUTPUT);
	}
	//
	pin_status[pin].mode = 0;
	pin_status[pin].value = 0;
	pin_status[pin].vextend = 0;
}

uint8_t binRecv(uint8_t argc, uint8_t *argv, uint8_t timeout) {
	if (argc) {
			return binConsole->read(binConsole, argv, argc, timeout);
	} else {
		if (binConsole->available(binConsole)) {
			return binConsole->read(binConsole, argv, 1, timeout);
		}
		return 0;
	}
}

uint8_t txtRecv(uint8_t argc, uint8_t *argv, uint8_t timeout) {
	if (argc) {
			return txtConsole->read(txtConsole, argv, argc, timeout);
	} else {
		if (txtConsole->available(txtConsole)) {
			return txtConsole->read(txtConsole, argv, 1, timeout);
		}
		return 0;
	}
}

uint8_t errRecv(uint8_t argc, uint8_t *argv, uint8_t timeout) {
	if (argc) {
			return errConsole->read(errConsole, argv, argc, timeout);
	} else {
		if (errConsole->available(errConsole)) {
			return errConsole->read(errConsole, argv, 1, timeout);
		}
		return 0;
	}
}

uint8_t peerRecv(uint8_t argc, uint8_t *argv, uint8_t timeout) {
	if (argc) {
			return peering->read(peering, argv, argc, timeout);
	} else {
		if (peering->available(peering)) {
			return peering->read(peering, argv, 1, timeout);
		}
		return 0;
	}
}

void binSend(uint8_t argc, uint8_t *argv) {
	binConsole->write(binConsole, argv, argc, 0);
}

void txtSend(uint8_t argc, uint8_t *argv) {
	txtConsole->write(txtConsole, argv, argc, 0);
}

void errSend(uint8_t argc, uint8_t *argv) {
	errConsole->write(errConsole, argv, argc, 0);
}

void peerSend(uint8_t argc, uint8_t *argv) {
	peering->write(peering, argv, argc, 0);
}

