#ifndef HAL_ARCH_H
#define HAL_ARCH_H

#ifdef __GIT_REVPARSE__
#define RELEASE_ARCH __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <hal/platform.h>
#include <inttypes.h>

// time keeping
extern volatile uint16_t tmicro;
extern volatile uint16_t tmilli;
extern volatile uint16_t tsecond;

// 1ms pulsing frame
// 	- 8 max pulsing pins (each pulse element is a 8 bit value, each bit represent 1 pin value)
// 	- 250 max pulses per millisecond (ie: 1 each 4us)
typedef struct pin_frame_s {
	volatile uint16_t milli;
	volatile uint8_t *pulse;
	volatile uint8_t pulse_counter;
	volatile struct pin_frame_s *next;
} pin_frame_t;
typedef struct pin_pulsing_s {
	volatile uint8_t pin[8];
	volatile pin_frame_t *head;
	volatile pin_frame_t *tail;
} pin_pulsing_t;


// pins
extern volatile uint8_t pin[PIN_TOTAL];
// adc
extern volatile uint8_t pin_adc_enable[16];
extern volatile uint8_t pin_adc_value[16];
extern volatile uint8_t pin_adc_current;
// pulsing pins
extern volatile pin_pulsing_t pin_pulsing;


// TIME
uint16_t cycles(void);
uint16_t micros(void);
uint16_t millis(void);
uint16_t seconds(void);
uint16_t uelapsed(uint16_t mstart, uint16_t ustart, uint16_t uend, uint16_t mend);

// PORTs
uint8_t port_read(uint8_t pin, uint8_t timeout);
uint8_t port_write(uint8_t port, uint8_t value);

// PINs
// Set pin to input/output mode.
void pin_mode(uint8_t pin, uint8_t mode);
// Return current pin state.
uint8_t pin_read(uint8_t pin, uint8_t timeout);
uint16_t pin_read_adc(uint8_t pin, uint8_t timeout);
// Set pin low(0). Shorthand for write(LOW).
void pin_set_low(uint8_t pin);
// Set pin high(1). Shorthand for write(HIGH).
void pin_set_high(uint8_t pin);
// Toggle pin state. Shorthand for write(!read()).
void pin_toggle(uint8_t pin);
// Set pin to given state. Non-zero value will set the pin HIGH, and zero value will set the pin LOW.
void pin_write(uint8_t pin, uint8_t value);
void pin_write_pwm(uint8_t pin, uint16_t value);
// Used with input() to activate internal pullup resistor on
void pin_pullup_enable(uint8_t pin);
// Open-drain pin. Use input() for high and output() for low.
void pin_open_drain(uint8_t pin);

// PULSING
// Detect a single pulse and return its width in micro-seconds.
uint16_t pin_pulse_detect(uint8_t pin);
// Generate a single pulse with given width in micro-seconds.
void pin_pulse_single(uint8_t pin, uint16_t width);
// Add 1 frame at 'milli(seconds)' to pulsing queue
volatile pin_frame_t* pin_pulse_multi(uint16_t milli);

// ARCH
void vars_reset(void);

// init MCU
void _arch_init(void); // arch specific
void arch_init(void); // common

// tasks to run every loop cycle
void _arch_run(void); // arch specific
void arch_run(void); // common

// set MCU to know state (ie: ready for init)
void _arch_reset(void); // arch specific
uint8_t arch_reset(void); // common

// halt MCU (ie: need to power cycle before init)
uint8_t arch_halt(void);

#ifdef __cplusplus
}
#endif

#endif

