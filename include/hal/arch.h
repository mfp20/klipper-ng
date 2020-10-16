
#ifndef HAL_ARCH_H
#define HAL_ARCH_H

#include <utility/bitops.h>
#include <utility/cbuffer.h>
#include <hal/defines.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

	// TYPEDEFs

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
	//
	typedef struct commport_s commport_t;
	struct commport_s {
		uint8_t type;
		uint8_t no;
		uint8_t fd;
		uint32_t baud;
		int (*begin)(commport_t *port, uint32_t baud);
		bool (*available)(commport_t *port);
		int (*read)(commport_t *port, uint8_t *data, uint8_t count, uint16_t timeout);
		int (*write)(commport_t *port, uint8_t *data, uint8_t count, uint16_t timeout);
		int (*end)(commport_t *port);
	};


	// VARs

	// time since boot
	extern volatile uint16_t tnano;
	extern volatile uint16_t tmicro;
	extern volatile uint16_t tmilli;
	extern volatile uint16_t tsecond;
	// adc
	extern volatile uint8_t adc_enable[16];
	extern volatile uint8_t adc_value[16];
	extern volatile uint8_t adc_current;
	// pulsing pins
	extern volatile pin_pulsing_t pulsing;
	//
	extern commport_t *ports;
	extern uint8_t ports_no;
	extern commport_t *console;


	// FUNCTIONs

	// TIME
	uint16_t nanos(void);
	uint16_t micros(void);
	uint16_t millis(void);
	uint16_t seconds(void);
	inline uint16_t elapsed(uint16_t mstart, uint16_t ustart, uint16_t uend, uint16_t mend) {
		// less than 1ms, within the same millisecond interval
		if (mstart==mend) {
			if (uend>ustart) {
				return uend-ustart;
			} else {
				return (1000-ustart)+uend;
			}
			// less than 1ms, across 2ms interval
		} else if (mend==mstart+1) {
			if (uend>ustart) {
				return (1000-ustart)+uend;
			} else {
				return (1000-ustart)+uend;
			}
			// more than 1ms
		} else {
			if (mend>mstart) {
				return ((mend-mstart-1)*1000)+((1000-ustart)+uend);
			} else {
				return (((1000-mstart)+mend+1)*1000)+((1000-ustart)+uend);
			}
		}
	}


	// PINS
	// Set pin to input/output mode.
	void pin_mode(uint8_t pin, uint8_t mode);
	// Return current pin state.
	uint8_t pin_read(uint8_t pin);
	// Set pin low(0). Shorthand for write(LOW).
	void pin_set_low(uint8_t pin);
	// Set pin high(1). Shorthand for write(HIGH).
	void pin_set_high(uint8_t pin);
	// Toggle pin state. Shorthand for write(!read()).
	void pin_toggle(uint8_t pin);
	// Set pin to given state. Non-zero value will set the pin HIGH, and zero value will set the pin LOW.
	void pin_write(uint8_t pin, uint8_t value);
	// Used with input() to activate internal pullup resistor on
	void pin_pullup_enable(uint8_t pin);
	// Open-drain pin. Use input() for high and output() for low.
	void pin_open_drain(uint8_t pin);
	// read an 8 bit port
	unsigned char pin_port_read(uint8_t port, uint8_t bitmask);
	// write an 8 bit port, only touch pins specified by a bitmask
	unsigned char pin_port_write(uint8_t port, uint8_t value, uint8_t bitmask);

	// PULSING
	// Detect a single pulse and return its width in micro-seconds.
	uint16_t pin_pulse_detect(uint8_t pin);
	// Generate a single pulse with given width in micro-seconds.
	void pin_pulse_single(uint8_t pin, uint16_t width);
	// Add 1 frame at 'milli(seconds)' to pulsing queue
	volatile pin_frame_t* pin_pulse_multi(uint16_t milli);


	// COMM PORTS
	// register a new commport_t
	commport_t* commport_register(uint8_t type, uint8_t no);
	// setup device and return the FD
	int uart_begin(commport_t *uart, uint32_t baud);
	// return 1 if data available to read, else 0
	bool uart_available(commport_t *uart);
	// read count bytes from data within timeout
	int uart_read(commport_t *uart, uint8_t *data, uint8_t count, uint16_t timeout);
	// write count bytes from data within timeout
	int uart_write(commport_t *uart, uint8_t *data, uint8_t count, uint16_t timeout);
	// close the device
	int uart_end(commport_t *uart);
	// TODO Same for: onewire, i2c, spi


	// ARCH
	static inline void vars_reset(void) {
		//
		for (uint8_t i = 0;i<8;i++) {
			adc_enable[i] = 0;
			adc_value[i] = 0;
		}
		//
		for (uint8_t i = 0;i<8;i++) {
			pulsing.pin[i] = 0;
		}
		pulsing.head = NULL;
		pulsing.tail = NULL;
		//printf("vars_reset() OK\n");
	}
	static inline void commports_reset(void) {
		if (ports != NULL)
			for (int i=0;i<=ports_no;i++) {
				if (ports[i].fd != 0) {
					//printf("port %d, fd %d", i, ports[i].fd);
					ports[i].end(&ports[i]);
				}
			}
		//if (ports != NULL) free(ports);
		ports = (commport_t *)realloc(ports, sizeof(commport_t));
		ports_no = 0;
		console = NULL;
		//printf("commports_reset() OK\n");
	}

	// console&stdout&stderr
	void consoleWrite(const char *format, ...);
	void logWrite(const char *format, ...);
	void errWrite(const char *format, ...);

	// init MCU
	void _arch_init(void);
	inline void arch_init(void) {
		vars_reset();
		commports_reset();
		// arch specific
		_arch_init();
		//printf("arch_init OK\n");
	}

	// tasks to run every loop cycle
	void _arch_run(void);
	inline void arch_run(void) {
		_arch_run();
	}

	// set MCU to know state (ie: ready for init)
	void _arch_reset(void);
	inline uint8_t arch_reset(void) {
		_arch_reset();
		commports_reset();
		vars_reset();
		return 0;
	}

	// halt MCU (ie: need to power cycle before init)
	inline uint8_t arch_halt(void) {
		arch_reset();
		return 0;
	}

#ifdef __cplusplus
}
#endif

#endif

