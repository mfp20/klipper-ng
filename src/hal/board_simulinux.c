// linux device simulator

#include <hal/board.h>
#include <stdlib.h>
#include <termios.h>	// termios
#include <pty.h>		// openpty
#include <fcntl.h>		// fcntl
#include <sys/ioctl.h>	// ioctl
#include <sys/stat.h>	// chmod
#include <stdio.h>		// sprintf
#include <string.h>		// memset, strcopy
#include <unistd.h>		// usleep, read, write, ...
#include <stdarg.h>		// va_arg

// FD
static const char *fd_pty_filename = "/tmp/klipper-ng-pty";
static uint8_t fd_pty_count = 0;
uint8_t fd_begin(commport_t *cp, uint32_t baud) {
	if (cp->id > 2) {
		// init termios
		struct termios ti;
		memset(&ti, 0, sizeof(ti));
		// open pseudo-tty
		int mfd, sfd;
		openpty(&mfd, &sfd, NULL, &ti, NULL);
		// set non-blocking
		int flags = fcntl(mfd, F_GETFL);
		fcntl(mfd, F_SETFL, flags | O_NONBLOCK);
		// set close on exec
		fcntl(mfd, F_SETFD, FD_CLOEXEC);
		fcntl(sfd, F_SETFD, FD_CLOEXEC);
		// create pty filename
		char fnid[3];
		sprintf(fnid, "%d", fd_pty_count);
		fd_pty_count++;
		uint8_t len = strlen(fd_pty_filename)+3;
		char fname[len];
		strcpy(fname, fd_pty_filename);
		strcat(fname, fnid);
		// create symlink to tty
		unlink(fname);
		char *tname = ttyname(sfd);
		if (symlink(tname, fname)) printf("symlink error");
		chmod(tname, 0660);
		// make sure stderr is non-blocking
		flags = fcntl(STDERR_FILENO, F_GETFL);
		fcntl(STDERR_FILENO, F_SETFL, flags | O_NONBLOCK);
		//
		printf("- PTY ready @ %s\n", fname);
		return mfd;
	} else {
		return cp->id;
	}
}

uint8_t fd_available(commport_t *cp) {
	//struct timeval tv;
	//tv.tv_sec = 0;
	//tv.tv_usec = 5;
	//select(NULL, &uart->fd, NULL, NULL, &tv);
	int count;
	ioctl(cp->fd, FIONREAD, &count);
	return count;
}

uint8_t fd_read(commport_t *cp, uint8_t *data, uint8_t count, uint16_t timeout) {
	read(cp->fd, data, count);
	return 0;
}

uint8_t fd_write(commport_t *cp, uint8_t *data, uint8_t count, uint16_t timeout) {
	write(cp->fd, data, count);
	return 0;
}

uint8_t fd_end(commport_t *cp) {
	// TODO
	fd_pty_count--;
	return 0;
}

commport_t* commport_register(uint8_t type, uint8_t no) {
	// allocate ports array
	port = (commport_t *)realloc(port, sizeof(commport_t)*(ports_no+1));
	// assign metods
	port[ports_no].type = type;
	port[ports_no].id = no;
	switch (type) {
		case COMMPORT_TYPE_FD:
			port[ports_no].pin = NULL;
			port[ports_no].baud = DEFAULT_BAUD;
			port[ports_no].begin = fd_begin;
			port[ports_no].available = fd_available;
			port[ports_no].read = fd_read;
			port[ports_no].write = fd_write;
			port[ports_no].end = fd_end;
			break;
		case COMMPORT_TYPE_TTY: // fake uart, creates a socket file
			port[ports_no].id = 3;
			port[ports_no].pin = &pins_tty[no];
			port[ports_no].baud = DEFAULT_BAUD;
			port[ports_no].begin = fd_begin;
			port[ports_no].available = fd_available;
			port[ports_no].read = fd_read;
			port[ports_no].write = fd_write;
			port[ports_no].end = fd_end;
			break;
	}
	port[ports_no].fd = port[ports_no].begin(&port[ports_no], DEFAULT_BAUD);
	if (ports_no == 0) console = &port[ports_no];
	ports_no++;
	return &port[ports_no-1];
}

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

void _board_init(void) {
	// assign pins to function arrays
	pins_pwm[0] = P1;
	pins_adc[0] = P2;
	for(int i=0;i<ONEWIRE_CH;i++) {
		pins_1wire[i] = pin[i];
	}
	pins_tty[0][0] = P1;
	pins_tty[0][1] = P2;
	pins_tty[1][0] = P4;
	pins_tty[1][1] = P5;
	pins_i2c[0][0] = P6;
	pins_i2c[0][1] = P7;
	pins_spi[0][0] = P3;
	pins_spi[0][1] = P4;
	pins_spi[0][2] = P5;
	pins_spi[0][3] = P6;

	// init default ports
	commport_register(COMMPORT_TYPE_TTY, 0);
	binConsole = &port[0];
	commport_register(COMMPORT_TYPE_FD, 1);
	txtConsole = &port[1];
	commport_register(COMMPORT_TYPE_FD, 2);
	errConsole = &port[2];
	peering = NULL;
}

void _board_run(void) {
}

void _board_reset(void) {
}

