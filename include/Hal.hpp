
#ifndef HAL_H
#define HAL_H

#include <inttypes.h> // for uint8_t
#include <stdio.h> // for size_t
#include <hal/arch.h>
#include <hal/board.h>

// HAL release number TODO: get git hash.
#define RELEASE_HAL  1

class SerialStream {
	public:
		SerialStream(void);
		SerialStream(commport_t *p);
		virtual void begin(unsigned long baud); // init port
		virtual int available(void); // data ready to be read
		virtual int read(void);
		virtual size_t write(uint8_t c);
		virtual void end(void); // close port
	private:
		commport_t *port;
};

class Hal {
	public:
		SerialStream *txtConsole; // serial channel for text messages and user interactive console
		SerialStream *binConsole; // serial channel for binary control messages
		SerialStream *peer; // serial channel for p2p messages with other MCUs (ex: sync)

		Hal(void);
		void init(void);
		uint16_t run(void);
};

#endif

