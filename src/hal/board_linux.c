// linux device (ex: generic x86, rpi, odroid, ...)

#include <hal/board.h>
#include <stdarg.h>		// va_arg
#include <termios.h>	// termios
#include <pty.h>		// openpty
#include <fcntl.h>		// fcntl
#include <sys/ioctl.h>	// ioctl
#include <sys/stat.h>	// chmod
#include <stdlib.h>		// sprintf
#include <stdio.h>		// sprintf
#include <string.h>		// memset, strcopy
#include <unistd.h>		// usleep, read, write, ...
#include <stdbool.h>	//

// COMMS -----------------------------------------------------------------------------------

commport_t* commport_register(uint8_t type, uint8_t no) {
	// allocate ports array
	port = (commport_t *)realloc(port, sizeof(commport_t)*(ports_no+1));
	// assign metods
	port[ports_no].type = type;
	port[ports_no].id = no;
	switch (type) {
		case COMMPORT_TYPE_FD:
			port[ports_no].pin = NULL;
			port[ports_no].fd = 0;
			port[ports_no].baud = DEFAULT_BAUD;
			//port[ports_no].begin = onewire_begin;
			//port[ports_no].available = onewire_available;
			//port[ports_no].read = onewire_read;
			//port[ports_no].write = onewire_write;
			//port[ports_no].end = onewire_end;
			break;
		case COMMPORT_TYPE_1WIRE:
			port[ports_no].pin = &pins_1wire[no];
			port[ports_no].fd = 0;
			port[ports_no].baud = DEFAULT_BAUD;
			//port[ports_no].begin = onewire_begin;
			//port[ports_no].available = onewire_available;
			//port[ports_no].read = onewire_read;
			//port[ports_no].write = onewire_write;
			//port[ports_no].end = onewire_end;
			break;
		case COMMPORT_TYPE_TTY:
			port[ports_no].pin = &pins_tty[no];
			port[ports_no].fd = 0;
			port[ports_no].baud = DEFAULT_BAUD;
			port[ports_no].begin = uart_begin;
			port[ports_no].available = uart_available;
			port[ports_no].read = uart_read;
			port[ports_no].write = uart_write;
			port[ports_no].end = uart_end;
			port[0].begin(&port[0], DEFAULT_BAUD);
			break;
		case COMMPORT_TYPE_I2C:
			port[ports_no].pin = &pins_i2c[no];
			port[ports_no].fd = 0;
			port[ports_no].baud = DEFAULT_BAUD;
			//port[ports_no].begin = i2c_begin;
			//port[ports_no].available = i2c_available;
			//port[ports_no].read = i2c_read;
			//port[ports_no].write = i2c_write;
			//port[ports_no].end = i2c_end;
			break;
		case COMMPORT_TYPE_SPI:
			port[ports_no].pin = &pins_spi[no];
			port[ports_no].fd = 0;
			port[ports_no].baud = DEFAULT_BAUD;
			//port[ports_no].begin = spi_begin;
			//port[ports_no].available = spi_available;
			//port[ports_no].read = spi_read;
			//port[ports_no].write = spi_write;
			//port[ports_no].end = spi_end;
			break;
	}
	if (ports_no == 0) console = &port[ports_no];
	ports_no++;
	return &port[ports_no-1];
}


// TODO


void _board_init(void) {
	binWrite = consoleWrite;
	txtWrite = stdoutWrite;
	errWrite = stdoutWrite;

	// TODO

	// init default console (port[0])
	commport_register(COMMPORT_TYPE_TTY, 0);
}

void _board_run(void) {
	// TODO
}

void _board_reset(void) {
	// TODO
}

