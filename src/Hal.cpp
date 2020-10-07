
#include <Hal.hpp>

SerialStream::SerialStream(void) {}
SerialStream::SerialStream(commport_t *p) {
	port = p;
}

void SerialStream::begin(unsigned long baud) {
	port->begin(port, baud);
}

int SerialStream::available(void) {
	return port->available(port);
}

int SerialStream::read(void) {
	uint8_t c;
	port->read(port, &c, 1, 1000);
	return c;
}

size_t SerialStream::write(uint8_t c) {
	return port->write(port, &c, 1, 1000);
}

void SerialStream::end(void) {
	port->end(port);
}


// ---

Hal::Hal(void) {}

void Hal::init(void) {
	arch_init();
	board_init();
	binConsole = new SerialStream(&ports[0]);
	txtConsole = binConsole;
	//printf("Hal::init OK\n");
}

uint16_t Hal::run(void) {
	return arch_run() + board_run();
}

