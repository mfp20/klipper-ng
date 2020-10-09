
#ifndef PROTOCOL_ENCODER7BIT_H
#define PROTOCOL_ENCODER7BIT_H

#include <inttypes.h>
#include <protocol/Firmata.hpp>

#define num7BitOutbytes(a)(((a)*7)>>3)

class Firmata7BitEnc {
	public:
		Firmata7BitEnc();
		void start(void);
		void write(uint8_t data);
		void end(void);
		void read(int outBytes, uint8_t *inData, uint8_t *outData);
	private:
		uint8_t previous;
		int shift;
};

extern Firmata firmata;

#endif

