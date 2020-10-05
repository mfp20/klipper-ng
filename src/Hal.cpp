
#include <Hal.hpp>

void SerialStream::begin(unsigned long baud) { baud= baud; } // init port
int SerialStream::available(void) { return 0; } // data ready to be read
int SerialStream::read(void) { return 0; }
size_t SerialStream::write(uint8_t) { return 0; }
void SerialStream::end() {} // close port

Hal hal;

void Hal::addCh(SerialStream *c) {
	c = c;
	// TODO: realloc All, copy c, increment ch_no
}

void Hal::listChs(void) {
	// TODO: list available channels
}
