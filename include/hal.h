#ifndef HAL_H
#define HAL_H

#ifdef __GIT_REVPARSE__
#define RELEASE_HAL __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <hal/board.h>

	typedef struct pin_status_s {
		volatile uint8_t mode; // refer to protocol for known modes
		volatile uint8_t value; // current value, MSB for 16bit values
		volatile uint8_t vextend; // LSB for 16bit values (ex: AVR ADC pins)
	} pin_status_t;

	// in-use pins status, array index is the pin number
	extern pin_status_t *pin_status;
	extern uint8_t pin_status_size;
	// preferred pins (16 preferred pins, for easy access)
	extern pin_status_t *preferred_pin[16];
	// pin groups (4 groups of 16 pins each, for easy access)
	extern pin_status_t *pin_group[4][16];

	//
	void halInit(void);
	void halRun(void);
	uint16_t halPause(uint16_t delay);
	uint16_t halResume(uint16_t time);
	void halReset(uint8_t mode);

	// pins
	uint8_t pinIsFree(uint8_t pin);
	void pinUse(uint8_t pin, uint8_t mode);
	void pinPrefer(uint8_t pin, uint8_t pos);
	void pinGroup(uint8_t pin, uint8_t group, uint8_t pos);
	void pinMode(uint8_t pin, uint8_t mode);
	uint8_t pinRead(uint8_t pin, uint8_t timeout);
	void pinWrite(uint8_t pin, uint8_t value);
	void pinFree(uint8_t pin);

	// comm ports
	uint8_t binRecv(uint8_t argc, uint8_t *argv, uint8_t timeout);
	uint8_t txtRecv(uint8_t argc, uint8_t *argv, uint8_t timeout);
	uint8_t errRecv(uint8_t argc, uint8_t *argv, uint8_t timeout);
	uint8_t peerRecv(uint8_t argc, uint8_t *argv, uint8_t timeout);
	void binSend(uint8_t argc, uint8_t *argv);
	void txtSend(uint8_t argc, uint8_t *argv);
	void errSend(uint8_t argc, uint8_t *argv);
	void peerSend(uint8_t argc, uint8_t *argv);

#ifdef __cplusplus
}
#endif

#endif

