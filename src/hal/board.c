#include <hal/board.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>		// sprintf
#include <stdarg.h>		// va_arg

uint8_t pins_pwm[PWM_CH];
uint8_t pins_adc[ADC_CH];
uint8_t pins_1wire[ONEWIRE_CH];
uint8_t pins_tty[TTY_CH][2];
uint8_t pins_i2c[I2C_CH][2];
uint8_t pins_spi[SPI_CH][4];

commport_t *port = NULL;
uint8_t ports_no = 0;
commport_t *binConsole = NULL;
commport_t *txtConsole = NULL;
commport_t *errConsole = NULL;
commport_t *peering = NULL;

void consolePrint(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf(buffer,format, args);
	binConsole->write(binConsole, (uint8_t*)buffer, len, 0);
	va_end (args);
}

void stdoutPrint(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf(buffer,format, args);
	txtConsole->write(txtConsole, (uint8_t*)buffer, len, 0);
	va_end (args);
}

void stderrPrint(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf(buffer,format, args);
	errConsole->write(errConsole, (uint8_t*)buffer, len, 0);
	va_end (args);
}

void peerPrint(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf(buffer,format, args);
	peering->write(peering, (uint8_t*)buffer, len, 0);
	va_end (args);
}

void commports_reset(void) {
	if (port != NULL)
		for (int i=0;i<=ports_no;i++) {
			if (port[i].fd != 0) {
				//printf("port %d, fd %d", i, ports[i].fd);
				port[i].end(&port[i]);
			}
		}
	port = (commport_t *)realloc(port, sizeof(commport_t));
	ports_no = 0;
	binConsole = NULL;
	txtConsole = NULL;
	errConsole = NULL;
	peering = NULL;
}

void board_init(void) {
	_board_init();
}

void board_run(void) {
	_board_run();
}

uint8_t board_reset(void) {
	_board_reset();
	return 0;
}

