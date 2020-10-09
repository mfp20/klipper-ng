
#include <hal.h>

pin_status_t *pin_status = NULL;
uint8_t pin_status_size = 0;

commport_t *txtConsole; // serial channel for text messages and user interactive console
commport_t *binConsole; // serial channel for binary control messages
commport_t *peer; // serial channel for p2p messages with other MCUs (ex: sync)

void hal_init(void) {
	arch_init();
	board_init();

	binConsole = &ports[0];
	txtConsole = binConsole;

	// register pin status
	switch(ports[0].type) {
		case PORT_TYPE_ONEWIRE:
			uint8_t last = max(ONEWIRE, LED);
			pin_status = (pin_status_t)malloc(sizeof(pin_status_t)*last);
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[ONEWIRE].mode = PIN_MODE_ONEWIRE;
			pin_status[LED].mode = PIN_MODE_DIGITAL_OUTPUT;
			break;
		case PORT_TYPE_UART:
			uint8_t last = max(UART0_RX, UART0_TX);
			last = max(last, LED);
			pin_status = (pin_status_t)malloc(sizeof(pin_status_t)*last);
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[UART0_RX] = PIN_MODE_SERIAL;
			pin_status[UART0_TX] = PIN_MODE_SERIAL;
			pin_status[LED].mode = PIN_MODE_DIGITAL_OUTPUT;
			break;
		case PORT_TYPE_I2C:
			uint8_t last = max(SCA, SCL);
			last = max(last, LED);
			pin_status = (pin_status_t)malloc(sizeof(pin_status_t)*last);
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[SCA].mode = PIN_MODE_I2C;
			pin_status[SCL].mode = PIN_MODE_I2C;
			pin_status[LED].mode = PIN_MODE_DIGITAL_OUTPUT;
			break;
		case PORT_TYPE_SPI:
			uint8_t last = max(SS, MOSI);
			last = max(last, MISO);
			last = max(last, SCK);
			last = max(last, LED);
			pin_status = (pin_status_t)malloc(sizeof(pin_status_t)*last);
			pin_status_size = last;
			for(uint8_t i=1;i<=pin_status_size;i++) {
				pin_status[i].mode = 0;
				pin_status[i].value = 0;
				pin_status[i].vextend = 0;
			}
			pin_status[SS].mode = PIN_MODE_SPI;
			pin_status[MOSI].mode = PIN_MODE_SPI;
			pin_status[MISO].mode = PIN_MODE_SPI;
			pin_status[SCK].mode = PIN_MODE_SPI;
			pin_status[LED].mode = PIN_MODE_DIGITAL_OUTPUT;
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

