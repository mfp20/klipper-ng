
#ifndef HAL_H
#define HAL_H

extern "C" {

#include <inttypes.h> // for uint8_t
#include <stdio.h> // for size_t
#include <hal/arch.h>
#include <hal/board.h>

	// pin modes
#define PIN_MODE_DIGITAL_INPUT	0x00 // 
#define PIN_MODE_DIGITAL_OUTPUT	0x01 // 
#define PIN_MODE_ANALOG_INPUT	0x02 // analog pin in analogInput mode
#define PIN_MODE_PWM            0x03 // digital pin in PWM output mode
#define PIN_MODE_SERVO          0x04 // digital pin in Servo output mode
#define PIN_MODE_SHIFT          0x05 // shiftIn/shiftOut mode
#define PIN_MODE_I2C            0x06 // pin included in I2C setup
#define PIN_MODE_ONEWIRE        0x07 // pin configured for 1-wire
#define PIN_MODE_STEPPER        0x08 // pin configured for stepper motor
#define PIN_MODE_ENCODER        0x09 // pin configured for rotary encoders
#define PIN_MODE_SERIAL         0x0A // pin configured for serial communication
#define PIN_MODE_PULLUP         0x0B // enable internal pull-up resistor for pin
#define PIN_MODE_SPI			0x0C // pin included in SPI setup
#define PIN_MODE_IGNORE         0x7F // pin configured to be ignored
#define TOTAL_PIN_MODES         14

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

	extern commport_t *txtConsole; // serial channel for text messages and user interactive console
	extern commport_t *binConsole; // serial channel for binary control messages
	extern commport_t *peer; // serial channel for p2p messages with other MCUs (ex: sync)

	void hal_init(void);
	uint16_t hal_run(void);

	void usePin(uint8_t pin, uint8_t mode); 
	void freePin(uint8_t pin);

}

#endif

