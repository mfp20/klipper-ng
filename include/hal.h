
#ifndef HAL_H
#define HAL_H

#include <hal/arch.h>
#include <hal/board.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h> // for uint8_t
#include <stdio.h> // for size_t

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })

	// in-use pins status, array index is the pin number
	typedef struct pin_status_s {
		volatile uint8_t mode; // refer to protocol for known modes
		volatile uint8_t value; // current value
		volatile uint8_t vextend; // LSB for 16bit values (ex: AVR ADC pins)
	} pin_status_t;

	extern pin_status_t *pin_status;
	extern uint8_t pin_status_size;

	extern commport_t *txtConsole; // serial port for text messages and user interactive console
	extern commport_t *binConsole; // serial port for binary control messages
	extern commport_t *peering; // serial port for p2p messages with other MCUs (ex: sync)

	void hal_init(void);
	uint16_t hal_run(void);

	void usePin(uint8_t pin, uint8_t mode);
	void freePin(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif

