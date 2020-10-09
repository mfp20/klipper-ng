
#include <protocol/Firmata7BitEnc.hpp>

Firmata7BitEnc::Firmata7BitEnc() {
	previous = 0;
	shift = 0;
}

void Firmata7BitEnc::start(void) {
	shift = 0;
}

void Firmata7BitEnc::write(uint8_t data) {
	if (shift == 0) {
		firmata.write(data & 0x7f);
		shift++;
		previous = data >> 7;
	} else {
		firmata.write(((data << shift) & 0x7f) | previous);
		if (shift == 6) {
			firmata.write(data >> 1);
			shift = 0;
		} else {
			shift++;
			previous = data >> (8 - shift);
		}
	}
}

void Firmata7BitEnc::end(void) {
	if (shift > 0) {
		firmata.write(previous);
	}
}

void Firmata7BitEnc::read(int outBytes, uint8_t *inData, uint8_t *outData) {
	for (int i = 0; i < outBytes; i++) {
		int j = i << 3;
		int pos = j / 7;
		uint8_t shift = j % 7;
		outData[i] = (inData[pos] >> shift) | ((inData[pos + 1] << (7 - shift)) & 0xFF);
	}
}

