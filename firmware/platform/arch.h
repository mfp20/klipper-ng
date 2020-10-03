#ifndef PLATFORM_ARCH_H
#define PLATFORM_ARCH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h> //
#include "../cbuffer.h"

/* BIT MACROS */

// creates a bit mask. note: the compiler will truncate the value to 16-bits max.
#define BIT(x) (0x01 << (x))
// creates a bit mask for unsigned long (32 bit). 
#define BITLONG(x) ((unsigned long)0x00000001 << (x))
// to get bit number 3 of 'foo', bit_get(foo, BIT(3))
#define BIT_GET(p,m) ((p) & (m))
// to set bit number 3 of 'foo', bit_set(foo, BIT(3))
#define BIT_SET(p,m) ((p) |= (m))
// to clear bit number 3 of 'foo', bit_clear(foo, BIT(3))
#define BIT_CLEAR(p,m) ((p) &= ~(m))
// to flip bit number 3 of 'foo', bit_flip(foo, BIT(3))
#define BIT_FLIP(p,m) ((p) ^= (m))
// to set or clear bar's bit 0, depending on foo's bit 4, bit_write(bit_get(foo, BIT(4)), bar, BIT(0))
#define BIT_WRITE(c,p,m) (c ? BIT_SET(p,m) : BIT_CLEAR(p,m))


/* TIME KEEPING */

// time since boot
volatile uint16_t tnano;
volatile uint16_t tmicro;
volatile uint16_t tmilli;
volatile uint16_t tsecond;

uint16_t nanos(void); 
uint16_t micros(void); 
uint16_t millis(void); 
uint16_t seconds(void);


/* PIN GET/SET */

// Set pin to input mode.
void pin_input_enable(uint8_t pin);
// Set pin to output mode.
void pin_output_enable(uint8_t pin);
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


/* ADC */

volatile uint8_t adc_enable[16];
volatile uint8_t adc_value[16];
volatile uint8_t adc_current;


/* PULSING */ 

// 1ms pulsing frame
// 	- 8 max pulsing pins (each pulse element is a 8 bit value, each bit represent 1 pin value)
// 	- 250 max pulses per millisecond (ie: 1 each 4us)
typedef struct pin_frame_s {
	volatile uint16_t milli;
	volatile uint8_t *pulse;
	volatile uint8_t pulse_counter;
	volatile struct pin_frame_s *next;
} pin_frame_t;
// pulsing pins&frames
typedef struct pin_pulsing_s {
	volatile uint8_t pin[8];
	volatile pin_frame_t *head;
	volatile pin_frame_t *tail;
} pin_pulsing_t;
// pulsing buffer
volatile pin_pulsing_t pulsing;

// Detect a single pulse and return its width in micro-seconds.
uint16_t pin_pulse_detect(uint8_t pin);
// Generate a single pulse with given width in micro-seconds.
void pin_pulse_single(uint8_t pin, uint16_t width);
// Add 1 frame at 'milli(seconds)' to pulsing queue
volatile pin_frame_t* pin_pulse_multi(uint16_t milli);


/* COMMS */

#define RX_BUFFER_SIZE 64 // 1,2,4,8,16,32,64,128 or 256 bytes
#define RX_BUFFER_MASK ( RX_BUFFER_SIZE - 1 )
#if ( RX_BUFFER_SIZE & RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#define TX_BUFFER_SIZE 64 // 1,2,4,8,16,32,64,128 or 256 bytes
#define TX_BUFFER_MASK ( TX_BUFFER_SIZE - 1 )
#if ( TX_BUFFER_SIZE & TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif
#define ERROR_NO 0 // no error
#define ERROR_FRAME 1 // Frame Error (FEn)
#define ERROR_DATAOVR 2 // Data OverRun (DORn)
#define ERROR_PARITY 3 // Parity Error (UPEn)
#define ERROR_BUFOVF 4 // Buffer overflow

typedef struct commport_s commport_t;
struct commport_s {
	uint8_t fd;
	cbuffer_t rx_buf;
	uint8_t rx_err;
	cbuffer_t tx_buf;
	uint8_t tx_err;
	bool (*need_rx)(commport_t *uart);
	uint8_t (*rx_byte)(commport_t *uart);
	uint8_t (*rx_all)(commport_t *uart, uint16_t timeout);
	bool (*need_tx)(commport_t *uart);
	uint8_t (*tx_byte)(commport_t *uart);
	uint8_t (*tx_all)(commport_t *uart, uint16_t timeout);
};
commport_t *commports[8];


// UART
// setup device and return its FD
uint8_t uart_enable(commport_t *uart, uint32_t baud);
// return 1 if the rx buffer is empty
bool uart_need_rx(commport_t *uart);
// read 1 byte from device and fill the buffer
uint8_t uart_rx_byte(commport_t *uart);
// read all available bytes from device and fill the buffer
uint8_t uart_rx_all(commport_t *uart, uint16_t timeout);
// return 1 if the tx buffer is full
bool uart_need_tx(commport_t *uart);
// write 1 byte from buffer to device
uint8_t uart_tx_byte(commport_t *uart);
// read all available bytes from buffer to device
uint8_t uart_tx_all(commport_t *uart, uint16_t timeout);
// close the device
void uart_disable(commport_t *uart);


// I2C
uint8_t i2c_enable(uint32_t baud);
bool i2c_need_rx(void);
uint8_t i2c_rx_byte(void);
uint8_t i2c_rx_all(void);
bool i2c_need_tx(void);
uint8_t i2c_tx_byte(void);
uint8_t i2c_tx_all(void);
void i2c_disable(void);

// SPI
uint8_t spi_enable(uint32_t baud);
bool spi_need_rx(void);
uint8_t spi_rx_byte(void);
uint8_t spi_rx_all(void);
bool spi_need_tx(void);
uint8_t spi_tx_byte(void);
uint8_t spi_tx_all(void);
void spi_disable(void);


// ARCH
void logger(const char *string);
void arch_init(void);
uint8_t arch_run(uint16_t snow, uint16_t mnow, uint16_t utimeout);
uint8_t arch_close(void);

#endif

