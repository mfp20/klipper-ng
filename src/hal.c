
#include <hal.h>

pin_status_t *pin_status = NULL;
uint8_t pin_status_size = 0;

commport_t *txtConsole; // serial channel for text messages and user interactive console
commport_t *binConsole; // serial channel for binary control messages
commport_t *peering; // serial channel for p2p messages with other MCUs (ex: sync)

void hal_init(void) {
	arch_init();
	board_init();

	binConsole = &ports[0];
	txtConsole = binConsole;

	// register pin status
	uint8_t last;
	switch(ports[0].type) {
		case COMMPORT_TYPE_1WIRE:
			last = max(PIN_1WIRE, PIN_LED);
			pin_status = (pin_status_t *)malloc(sizeof(pin_status_t)*last);
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
			last = max(PIN_UART0_RX, PIN_UART0_TX);
			last = max(last, PIN_LED);
			pin_status = (pin_status_t *)malloc(sizeof(pin_status_t)*last);
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
			last = max(PIN_SDA, PIN_SCL);
			last = max(last, PIN_LED);
			pin_status = (pin_status_t *)malloc(sizeof(pin_status_t)*last);
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
			last = max(PIN_SS, PIN_MOSI);
			last = max(last, PIN_MISO);
			last = max(last, PIN_SCK);
			last = max(last, PIN_LED);
			pin_status = (pin_status_t *)malloc(sizeof(pin_status_t)*last);
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

uint16_t hal_run(void) {
	return arch_run() + board_run();
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

