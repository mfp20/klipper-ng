
#ifndef HAL_H
#define HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h> // for uint8_t
#include <stdio.h> // for size_t
#include <hal/arch.h>
#include <hal/board.h>

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })

	// in-use pins status, array index is the pin number
	typedef struct pin_status_s {
		volatile uint8_t mode; // refer to protocol for known modes
		volatile uint8_t value; // current value, MSB for 16bit values
		volatile uint8_t vextend; // LSB for 16bit values (ex: AVR ADC pins)
	} pin_status_t;

	extern pin_status_t *pin_status;
	extern uint8_t pin_status_size;

	extern commport_t *binConsole; // serial port for binary control messages
	extern commport_t *txtConsole; // serial port for text messages and user interactive console
	extern commport_t *peering; // serial port for p2p messages with other MCUs (ex: sync)

	void halInit(void);
	void halRun(void);

	void usePin(uint8_t pin, uint8_t mode);
	void freePin(uint8_t pin);

	uint8_t binRecv(uint8_t argc, uint8_t *argv, uint8_t timeout);
	uint8_t txtRecv(uint8_t argc, uint8_t *argv, uint8_t timeout);
	uint8_t peerRecv(uint8_t argc, uint8_t *argv, uint8_t timeout);

	void binSend(uint8_t argc, uint8_t *argv);
	void txtSend(uint8_t argc, uint8_t *argv);
	void peerSend(uint8_t argc, uint8_t *argv);

#ifdef __cplusplus
}
#endif

#endif

