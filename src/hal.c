
#include <hal.h>

pin_status_t *pin_status = NULL;
uint8_t pin_status_size = 0;

commport_t *binConsole; // serial channel for binary control messages
commport_t *txtConsole; // serial channel for text messages and user interactive console
commport_t *peering; // serial channel for p2p messages with other MCUs (ex: sync)

void halInit(void) {
	arch_init();
	board_init();

	binConsole = &ports[0];

	// register pin status
	uint8_t last;
	switch(ports[0].type) {
		case COMMPORT_TYPE_1WIRE:
			last = MAX(PIN_1WIRE, PIN_LED);
			pin_status = (pin_status_t *)calloc(last+1, sizeof(pin_status_t));
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[PIN_1WIRE].mode = PIN_MODE_1WIRE;
			pin_status[PIN_LED].mode = PIN_MODE_DIGITAL_OUTPUT;
			break;
		case COMMPORT_TYPE_UART:
			last = MAX(PIN_UART0_RX, PIN_UART0_TX);
			last = MAX(last, PIN_LED);
			pin_status = (pin_status_t *)calloc(last+1, sizeof(pin_status_t));
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[PIN_UART0_RX].mode = PIN_MODE_SERIAL;
			pin_status[PIN_UART0_TX].mode = PIN_MODE_SERIAL;
			pin_status[PIN_LED].mode = PIN_MODE_DIGITAL_OUTPUT;
			break;
		case COMMPORT_TYPE_I2C:
			last = MAX(PIN_SDA, PIN_SCL);
			last = MAX(last, PIN_LED);
			pin_status = (pin_status_t *)calloc(last+1, sizeof(pin_status_t));
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[PIN_SDA].mode = PIN_MODE_I2C;
			pin_status[PIN_SCL].mode = PIN_MODE_I2C;
			pin_status[PIN_LED].mode = PIN_MODE_DIGITAL_OUTPUT;
			break;
		case COMMPORT_TYPE_SPI:
			last = MAX(PIN_SS, PIN_MOSI);
			last = MAX(last, PIN_MISO);
			last = MAX(last, PIN_SCK);
			last = MAX(last, PIN_LED);
			pin_status = (pin_status_t *)calloc(last+1, sizeof(pin_status_t));
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[PIN_SS].mode = PIN_MODE_SPI;
			pin_status[PIN_MOSI].mode = PIN_MODE_SPI;
			pin_status[PIN_MISO].mode = PIN_MODE_SPI;
			pin_status[PIN_SCK].mode = PIN_MODE_SPI;
			pin_status[PIN_LED].mode = PIN_MODE_DIGITAL_OUTPUT;
			break;
	}
	//printf("Hal::init OK\n");
}

void halRun(void) {
	arch_run();
	board_run();
}

void usePin(uint8_t pin, uint8_t mode) {
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

void freePin(uint8_t pin) {
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

void peerSend(uint8_t argc, uint8_t *argv) {
	peering->write(peering, argv, argc, 0);
}

