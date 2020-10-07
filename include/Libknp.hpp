
#ifndef LIBKNP_H
#define LIBKNP_H

#include <protocol/Firmata.hpp>
#include <protocol/FirmataExt.hpp>
#include <protocol/FirmataReporting.hpp>
#include <protocol/Firmata7BitEnc.hpp>
#include <protocol/FirmataScheduler.hpp>

extern Firmata firmata;
extern FirmataExt fExt;
extern FirmataReporting fRep;
extern Firmata7BitEnc fEnc;
extern FirmataScheduler fSched;

extern "C" {
	void init(void);
	uint16_t run(uint16_t utimeout);
	void systemResetCallback(void);
}

#endif

