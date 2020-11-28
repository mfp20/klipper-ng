// linux host

#include <hal/board.h>
#include <utility/fdthread.linux.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// FD
uint8_t fd_begin(commport_t *cp, uint32_t baud) {
	char *fname;
	if (cp->id == 0) {
		uint8_t len = strlen("/dev/stdin");
		fname = calloc(len,sizeof(char));
		strcpy(fname, "/dev/stdin");
		cp->id = fdOpen(fname,FD_TYPE_FILE);
	} else if (cp->id == 1) {
		uint8_t len = strlen("/dev/stdout");
		fname = calloc(len,sizeof(char));
		strcpy(fname, "/dev/stdout");
		cp->id = fdOpen(fname,FD_TYPE_FILE);
	} else if (cp->id == 2) {
		uint8_t len = strlen("/dev/stderr");
		fname = calloc(len,sizeof(char));
		strcpy(fname, "/dev/stderr");
		cp->id = fdOpen(fname,FD_TYPE_FILE);
	} else {
		char fnid[10];
		sprintf(fnid, "%d", fd_idx);
		fd_idx++;
		uint8_t len = strlen(fd_pty_filename)+strlen(fnid);
		fname = calloc(len,sizeof(char));
		strcpy(fname, fd_pty_filename);
		strcat(fname, fnid);
		cp->id = fdOpen(fname,FD_TYPE_PTY);
	}
	//
	free(fname);
	cp->fd = fdGet(cp->id);
	return cp->fd;
}

uint8_t fd_available(commport_t *cp) {
	return fdAvailable(cp->id);
}

uint8_t fd_read(commport_t *cp, uint8_t *data, uint8_t count, uint16_t timeout) {
	return fdRead(cp->id,data);
}

uint8_t fd_write(commport_t *cp, uint8_t *data, uint8_t count, uint16_t timeout) {
	return fdWrite(cp->id,data,count);
}

uint8_t fd_end(commport_t *cp) {
	return fdClose(cp->id);
}

uint8_t tty_begin(commport_t *cp, uint32_t baud) {
	return 0;
}

uint8_t tty_available(commport_t *cp) {
	return 0;
}

uint8_t tty_read(commport_t *cp, uint8_t *data, uint8_t count, uint16_t timeout) {
	return 0;
}

uint8_t tty_write(commport_t *cp, uint8_t *data, uint8_t count, uint16_t timeout) {
	return 0;
}

uint8_t tty_end(commport_t *cp) {
	return 0;
}

commport_t* commport_register(uint8_t type, uint8_t no) {
	// allocate ports array
	port = (commport_t *)realloc(port, sizeof(commport_t)*(ports_no+1));
	//
	port[ports_no].type = type;
	port[ports_no].id = no;
	port[ports_no].fd = 0;
	port[ports_no].pin = NULL;
	port[ports_no].baud = DEFAULT_BAUD;
	port[ports_no].begin = fd_begin;
	port[ports_no].available = fd_available;
	port[ports_no].read = fd_read;
	port[ports_no].write = fd_write;
	port[ports_no].end = fd_end;
	ports_no++;
	return &port[ports_no-1];
}

void _board_init(void) {
	// init FD thread
	initFdThread();
	// init default ports
	commport_register(COMMPORT_TYPE_TTY, 3);
	port[0].begin(&port[0], DEFAULT_BAUD);
	binConsole = &port[0];
	commport_register(COMMPORT_TYPE_TTY, 1);
	port[1].begin(&port[1], DEFAULT_BAUD);
	txtConsole = &port[1];
	commport_register(COMMPORT_TYPE_TTY, 2);
	port[2].begin(&port[2], DEFAULT_BAUD);
	errConsole = &port[2];
	peering = NULL;
}

void _board_run(void) {
}

void _board_reset(void) {
	closeFdThread();
}

