
#ifndef HAL_H
#define HAL_H

#include <inttypes.h> // for uint8_t
#include <stdio.h> // for size_t

class SerialStream {
	public:
		virtual void begin(unsigned long baud); // init port
		virtual int available(void); // data ready to be read
		virtual int read(void);
		virtual size_t write(uint8_t);
		virtual void end(); // close port
};

class Hal {
	public:
		SerialStream *Console; // for text messages and user interactive console
		SerialStream *CommandConsole; // for binary control messages
		SerialStream *Peer; // for p2p messages with other MCUs (ex: sync)
		void addCh(SerialStream *c);
		void listChs(void);
	private:
		SerialStream *All; // all available serial channels
		uint8_t ch_no;
};

extern Hal hal;

#endif

