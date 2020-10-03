// AVR Costs:
// - reg/int8 assignment/increment, 1 cycle
// - ISR, 4 cycles to enter the ISR and another 4 for reti
// - function call, 5 cycles
// ...

#include <avr/io.h>
#include <avr/interrupt.h>
#include "arch.h"


// GPIO -----------------------------------------------------------------------------------------------

// Pin values are bit-pointers and constructed from port control register address and pin bit position.
#define GPIO_PIN(port,pin) (((port) << 4) | (pin))
// Return io port control register address from pin value.
#define GPIO_REG(pin) ((pin) >> 4)
// Return pin mask from pin value.
#define GPIO_MASK(pin) _BV((pin) & 0xf)
// Maximum port control register address for atomic bit instructions.
#define GPIO_ATOMIC_MAX GPIO_PIN(0x40,0)
// Forces given expression to be atomic.
// Higher port addresses cannot be accessed with a single instruction 
// and require disabling of interrupts to become atomic.
#define GPIO_ATOMIC(pin, expr)					\
  do {								\
    if (pin < GPIO_ATOMIC_MAX) {				\
      expr;							\
    }								\
    else {							\
      uint8_t sreg = SREG;					\
      __asm__ __volatile__("cli" ::: "memory");			\
      expr;							\
      SREG = sreg;						\
      __asm__ __volatile__("" ::: "memory");			\
    }								\
  } while (0)

// The pin_t address is a bit pointer to the port control register.
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
// GPIO digital pin symbols for ATmega168/ATmega328P based boards.

// cpu@16Mhz: we have 1 cycle each 62.5ns. 16 cycles per 1us.
#define F_CPU 16000000UL  // 16 MHz

enum pin_t {
    P1 = GPIO_PIN(0x23,0),	//!< PINB:0
    P2 = GPIO_PIN(0x23,1),	//!< PINB:1
    P3 = GPIO_PIN(0x23,2),	//!< PINB:2/SS
    P4 = GPIO_PIN(0x23,3),	//!< PINB:3/MOSI/ICSP.4
    P5 = GPIO_PIN(0x23,4),	//!< PINB:4/MISO/ICSP.1
    P6 = GPIO_PIN(0x23,5),	//!< PINB:5/SCK/ICSP.3
    P7 = NULL,
    P8 = NULL,

    P9 = GPIO_PIN(0x26,0),	//!< PINC:0/A0
    P10 = GPIO_PIN(0x26,1),	//!< PINC:1/A1
    P11 = GPIO_PIN(0x26,2),	//!< PINC:2/A2
    P12 = GPIO_PIN(0x26,3),	//!< PINC:3/A3
    P13 = GPIO_PIN(0x26,4),	//!< PINC:4/A4/SDA
    P14 = GPIO_PIN(0x26,5),	//!< PINC:5/A5/SCL
    P15 = NULL,
    P16 = NULL,

    P17 = GPIO_PIN(0x29,0),	//!< PIND:0
    P18 = GPIO_PIN(0x29,1),	//!< PIND:1
    P19 = GPIO_PIN(0x29,2),	//!< PIND:2
    P20 = GPIO_PIN(0x29,3),	//!< PIND:3
    P21 = GPIO_PIN(0x29,4),	//!< PIND:4
    P22 = GPIO_PIN(0x29,5),	//!< PIND:5
    P23 = GPIO_PIN(0x29,6),	//!< PIND:6
    P24 = GPIO_PIN(0x29,7),	//!< PIND:7

    SS = P3,			//!< SPI Slave Select
    MOSI = P4,			//!< SPI Master Output Slave Input
    MISO = P5,			//!< SPI Master Input Slave Output
    SCK = P6,			//!< SPI Clock

    SDA = P13,			//!< TWI Data
    SCL = P14			//!< TWI Clock
};
#elif defined(__AVR_ATmega32U4__)
// GPIO digital pin symbols for ATmega32U4 based boards.
enum pin_t {
    P1 = GPIO_PIN(0x23,0),	//!< PINB:0/SS/RXLED
    P2 = GPIO_PIN(0x23,1),	//!< PINB:1/SCK/ICSP.3
    P3 = GPIO_PIN(0x23,2),	//!< PINB:2/MOSI/ICSP.4
    P4 = GPIO_PIN(0x23,3),	//!< PINB:3/MISO/ICSP.1
    P5 = GPIO_PIN(0x23,4),	//!< PINB:4
    P6 = GPIO_PIN(0x23,5),	//!< PINB:5
    P7 = GPIO_PIN(0x23,6),	//!< PINB:6
    P8 = GPIO_PIN(0x23,7),	//!< PINB:7

    P9 = NULL,
    P10 = NULL,
    P11 = NULL,
    P12 = NULL,
    P13 = NULL,
    P14 = NULL,
    P15 = GPIO_PIN(0x26,6),	//!< PINC:6
    P16 = GPIO_PIN(0x26,7),	//!< PINC:7

    P17 = GPIO_PIN(0x29,0),	//!< PIND:0/SCL
    P18 = GPIO_PIN(0x29,1),	//!< PIND:1/SDA
    P19 = GPIO_PIN(0x29,2),	//!< PIND:2
    P20 = GPIO_PIN(0x29,3),	//!< PIND:3
    P21 = GPIO_PIN(0x29,4),	//!< PIND:4
    P22 = NULL,
    P23 = GPIO_PIN(0x29,6),	//!< PIND:6
    P24 = GPIO_PIN(0x29,7),	//!< PIND:7

    P25 = NULL,
    P26 = NULL,
    P27 = NULL,
    P28 = NULL,
    P29 = NULL,
    P30 = NULL,
    D31 = GPIO_PIN(0x2c,6),	//!< PINE:6
    P32 = NULL,
 
    P33 = GPIO_PIN(0x2f,0),	//!< PINF:0/A5
    P34 = GPIO_PIN(0x2f,1),	//!< PINF:1/A4
    P35 = NULL,
    P36 = NULL,
    P37 = GPIO_PIN(0x2f,4),	//!< PINF:4/A3
    P38 = GPIO_PIN(0x2f,5),	//!< PINF:5/A2
    P39 = GPIO_PIN(0x2f,6),	//!< PINF:6/A1
    P40 = GPIO_PIN(0x2f,7),	//!< PINF:7/A0


    SS = P1,			//!< SPI Slave Select
    MOSI = P3,			//!< SPI Master Output Slave Input
    MISO = P4,			//!< SPI Master Input Slave Output
    SCK = P2,			//!< SPI Clock

    SDA = P18,			//!< TWI Data
    SCL = P17			//!< TWI Clock
};
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// GPIO digital pin symbols for ATmega1280/ATmega2560 based boards.

// cpu@16Mhz: we have 1 cycle each 62.5ns. 16 cycles per 1us.
#define F_CPU 16000000UL  // 16 MHz

enum pin_t {
    P1 = GPIO_PIN(0x20,0),	//!< PINA:0
    P2 = GPIO_PIN(0x20,1),	//!< PINA:1
    P3 = GPIO_PIN(0x20,2),	//!< PINA:2
    P4 = GPIO_PIN(0x20,3),	//!< PINA:3
    P5 = GPIO_PIN(0x20,4),	//!< PINA:4
    P6 = GPIO_PIN(0x20,5),	//!< PINA:5
    P7 = GPIO_PIN(0x20,6),	//!< PINA:6
    P8 = GPIO_PIN(0x20,7),	//!< PINA:7

    P9 = GPIO_PIN(0x23,0),	//!< PINB:0/SS
    P10 = GPIO_PIN(0x23,1),	//!< PINB:1/SCK/ICSP.4
    P11 = GPIO_PIN(0x23,2),	//!< PINB:2/MOSI/ICSP.3
    P12 = GPIO_PIN(0x23,3),	//!< PINB:3/MISO/ICSP.1
    P13 = GPIO_PIN(0x23,4),	//!< PINB:4
    P14 = GPIO_PIN(0x23,5),	//!< PINB:5
    P15 = GPIO_PIN(0x23,6),	//!< PINB:6
    P16 = GPIO_PIN(0x23,7),	//!< PINB:7

    P17 = GPIO_PIN(0x26,0),	//!< PINC:0
    P18 = GPIO_PIN(0x26,1),	//!< PINC:1
    P19 = GPIO_PIN(0x26,2),	//!< PINC:2
    P20 = GPIO_PIN(0x26,3),	//!< PINC:3
    P21 = GPIO_PIN(0x26,4),	//!< PINC:4
    P22 = GPIO_PIN(0x26,5),	//!< PINC:5
    P23 = GPIO_PIN(0x26,6),	//!< PINC:6
    P24 = GPIO_PIN(0x26,7),	//!< PINC:7

    P25 = GPIO_PIN(0x29,0),	//!< PIND:0
    P26 = GPIO_PIN(0x29,1),	//!< PIND:1/SDA
    P27 = GPIO_PIN(0x29,2),	//!< PIND:2/SCL
    P28 = GPIO_PIN(0x29,3),	//!< PIND:3
    P29 = NULL,
    P30 = NULL,
    P31 = NULL,
    P32 = GPIO_PIN(0x29,7),	//!< PIND:7

    P33 = GPIO_PIN(0x2c,0),	//!< PINE:0
    P34 = GPIO_PIN(0x2c,1),	//!< PINE:1
    P35 = NULL,
    P36 = GPIO_PIN(0x2c,3),	//!< PINE:3
    P37 = GPIO_PIN(0x2c,4),	//!< PINE:4
    P38 = GPIO_PIN(0x2c,5),	//!< PINE:5
    P39 = NULL,
    P40 = NULL,

    P41 = GPIO_PIN(0x2f,0),	//!< PINF:0/A0
    P42 = GPIO_PIN(0x2f,1),	//!< PINF:1/A1
    P43 = GPIO_PIN(0x2f,2),	//!< PINF:2/A2
    P44 = GPIO_PIN(0x2f,3),	//!< PINF:3/A3
    P45 = GPIO_PIN(0x2f,4),	//!< PINF:4/A4
    P46 = GPIO_PIN(0x2f,5),	//!< PINF:5/A5
    P47 = GPIO_PIN(0x2f,6),	//!< PINF:6/A6
    P48 = GPIO_PIN(0x2f,7),	//!< PINF:7/A7

    P49 = GPIO_PIN(0x32,0),	//!< PING:0
    P50 = GPIO_PIN(0x32,1),	//!< PING:1
    P51 = GPIO_PIN(0x32,2),	//!< PING:2
    P52 = NULL,
    P53 = NULL,
    P54 = GPIO_PIN(0x32,5),	//!< PING:5
    P55 = NULL,
    P56 = NULL,

    P57 = GPIO_PIN(0x100,0),	//!< PINH:0
    P58 = GPIO_PIN(0x100,1),	//!< PINH:1
    P59 = NULL,
    P60 = GPIO_PIN(0x100,3),	//!< PINH:3
    P61 = GPIO_PIN(0x100,4),	//!< PINH:4
    P62 = GPIO_PIN(0x100,5),	//!< PINH:5
    P63 = GPIO_PIN(0x100,6),	//!< PINH:6
    P64 = NULL,

    P65 = GPIO_PIN(0x106,0),	//!< PINK:0/A8
    P66 = GPIO_PIN(0x106,1),	//!< PINK:1/A9
    P67 = GPIO_PIN(0x106,2),	//!< PINK:2/A10
    P68 = GPIO_PIN(0x106,3),	//!< PINK:3/A11
    P69 = GPIO_PIN(0x106,4),	//!< PINK:4/A12
    P70 = GPIO_PIN(0x106,5),	//!< PINK:5/A13
    P71 = GPIO_PIN(0x106,6),	//!< PINK:6/A14
    P72 = GPIO_PIN(0x106,7),	//!< PINK:7/A15

    P73 = GPIO_PIN(0x103,0),	//!< PINJ:0
    P74 = GPIO_PIN(0x103,1),	//!< PINJ:1
    P75 = GPIO_PIN(0x103,2),	//!< PINJ:2
    P76 = GPIO_PIN(0x103,3),	//!< PINJ:3
    P77 = GPIO_PIN(0x103,4),	//!< PINJ:4
    P78 = GPIO_PIN(0x103,5),	//!< PINJ:5
    P79 = GPIO_PIN(0x103,6),	//!< PINJ:6
    P80 = GPIO_PIN(0x103,7),	//!< PINJ:7

    P81 = GPIO_PIN(0x109,0),	//!< PINL:0
    P82 = GPIO_PIN(0x109,1),	//!< PINL:1
    P83 = GPIO_PIN(0x109,2),	//!< PINL:2
    P84 = GPIO_PIN(0x109,3),	//!< PINL:3
    P85 = GPIO_PIN(0x109,4),	//!< PINL:4
    P86 = GPIO_PIN(0x109,5),	//!< PINL:5
    P87 = GPIO_PIN(0x109,6),	//!< PINL:6
    P88 = GPIO_PIN(0x109,7),	//!< PINL:7

    SS = P9,			//!< SPI Slave Select
    MOSI = P11,			//!< SPI Master Output Slave Input
    MISO = P12,			//!< SPI Master Input Slave Output
    SCK = P10,			//!< SPI Clock

    SDA = P26,			//!< TWI Data
    SCL = P27			//!< TWI Clock
};
#elif defined(__AVR_ATtiny24__)						\
 ||   defined(__AVR_ATtiny44__)						\
 ||   defined(__AVR_ATtiny84__)
// GPIO digital pin symbols for ATtinyX4 based boards.
enum pin_t {
    P1 = GPIO_PIN(0x39,0),	//!< PINA:0/A0
    P2 = GPIO_PIN(0x39,1),	//!< PINA:1/A1
    P3 = GPIO_PIN(0x39,2),	//!< PINA:2/A2
    P4 = GPIO_PIN(0x39,3),	//!< PINA:3/A3/SS
    P5 = GPIO_PIN(0x39,4),	//!< PINA:4/A4/SCL/SCK
    P6 = GPIO_PIN(0x39,5),	//!< PINA:5/A5/MOSI
    P7 = GPIO_PIN(0x39,6),	//!< PINA:6/A6/MISO/SDA

    P8 = GPIO_PIN(0x36,0),	//!< PINB:0
    P9 = GPIO_PIN(0x36,1),	//!< PINB:1
    P10 = GPIO_PIN(0x36,2),	//!< PINB:2
    P11 = NULL,
    P12 = NULL,
    P13 = NULL,
    P14 = NULL,
    P15 = NULL,
    P16 = GPIO_PIN(0x39,7),	//!< PINA:7/A7

    SS = P4,			//!< SPI Slave Select
    MOSI = P6,			//!< SPI Master Output Slave Input
    MISO = P7,			//!< SPI Master Input Slave Output
    SCK = P5			//!< SPI Clock
};
#elif defined(__AVR_ATtiny25__)						\
 ||   defined(__AVR_ATtiny45__)						\
 ||   defined(__AVR_ATtiny85__)
// GPIO digital pin symbols for ATtinyX5 based boards.
enum pin_t {
    P1 = GPIO_PIN(0x36,0),	//!< PINB:0/SDA/MISO
    P2 = GPIO_PIN(0x36,1),	//!< PINB:1/MOSI
    P3 = GPIO_PIN(0x36,2),	//!< PINB:2/A1/SCL/SCK
    P4 = GPIO_PIN(0x36,3),	//!< PINB:3/A3/SS
    P5 = GPIO_PIN(0x36,4),	//!< PINB:4/A2
    P6 = GPIO_PIN(0x36,5),	//!< PINB:5/A0
    P7 = NULL,
    P8 = NULL,

    SS = P4,			//!< SPI Slave Select
    MOSI = P2,			//!< SPI Master Output Slave Input
    MISO = P1,			//!< SPI Master Input Slave Output
    SCK = P3			//!< SPI Clock
};
#else
#error arch_avr.h: avr mcu not supported
#endif

// General Purpose Digital I/O Control Registers.
struct gpio_reg_t {
	volatile uint8_t pins;	//!< Port Input Pins.
	volatile uint8_t ddr;	//!< Data Direction Register.
	volatile uint8_t port;	//!< Data Register.
};

// Return pointer to control registers.
gpio_reg_t* SFR(pin_t pin)
__attribute__((always_inline)) {
	return ((gpio_reg_t*) GPIO_REG(pin));
}

void pin_input_enable(pin_t pin)
__attribute__((always_inline)) {
	GPIO_ATOMIC(pin, SFR(pin)->ddr &= ~GPIO_MASK(pin));
}

void pin_output_enable(pin_t pin)
__attribute__((always_inline)) {
	GPIO_ATOMIC(pin, SFR(pin)->ddr |= GPIO_MASK(pin));
}

bool pin_read(pin_t pin)
__attribute__((always_inline)) {
    return ((SFR(pin)->pins & GPIO_MASK(pin)) != 0);
}

void pin_set_low(pin_t pin)
__attribute__((always_inline)) {
	GPIO_ATOMIC(pin, SFR(pin)->port &= ~GPIO_MASK(pin));
}

void pin_set_high(pin_t pin)
__attribute__((always_inline)) {
	GPIO_ATOMIC(pin, SFR(pin)->port |= GPIO_MASK(pin));
}

void pin_toggle(pin_t pin)
__attribute__((always_inline)) {
	GPIO_ATOMIC(pin, SFR(pin)->pins |= GPIO_MASK(pin));
}

void pin_write(pin_t pin, int value)
__attribute__((always_inline)) {
	if (pin) if (value) pin_set_high(pin); else pin_set_low(pin);
}

void pin_pullup_enable(pin_t pin)
__attribute__((always_inline)) {
	pin_set_high(pin);
}

void pin_open_drain(pin_t pin)
__attribute__((always_inline)) {
    pin_input_enable(pin);
    pin_set_low(pin);
}


// TIME -----------------------------------------------------------------------------------

// cpu cycles counter
volatile uint16_t cycle = 0;
// overflow flags
volatile uint8_t t1_ovf = 0;
volatile uint8_t cycle_ovf = 0;

static void cycles_start(void) 
__attribute__((always_inline)) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		cycle = (uint16_t)TCNT1;
		cycle_ovf = t1_ovf;
	}
}
static uint16_t cycles_report(void) 
__attribute__((always_inline)) {
	uint16_t cs;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if ((t1_ovf)&&(!(cycle_ovf))) {
			cs = (15999-cycle)+TCNT1;
		} else {
			cs = TCNT1-cycle;
		}
	}
	return cs;
}
static uint16_t cycles_end(void) 
__attribute__((always_inline)) {
	uint16_t cs;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if ((t1_ovf)&&(!(cycle_ovf))) {
			t1_ovf = 0;
			cs = (15999-cycle)+TCNT1;
		} else {
			cs = TCNT1-cycle;
		}
	}
	cycle_ovf = 0;
	return cs;
}

uint16_t nanos(void) 
__attribute__((always_inline)) {
	uint16_t ticks;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		ticks = (uint16_t)TCNT1;
	}
	return (ticks*62.5);
}

uint16_t micros(void) 
__attribute__((always_inline)) {
	uint16_t ticks;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		ticks = (uint16_t)TCNT1;
	}
	return ((ticks*62.5)/1000);
}

uint16_t millis(void) 
__attribute__((always_inline)) {
	uint16_t ms;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if (milli>=999) {
			milli = milli - 999;
			timestamp++;
		}
		ms = milli;
	}
	return ms;
}

// 
uint16_t seconds(void) 
__attribute__((always_inline)) {
	uint16_t ticks;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if (milli>=999) {
			milli = milli - 999;
			timestamp++;
		}
		ticks = timestamp;
	}
	return ticks;
}


// ADC -----------------------------------------------------------------------------------------------

static void pin_input_adc_run(void) {
	// ADC, if the ADSC bit has been cleared, result is ready
	if(ADCSRA & (0 << ADSC)) {
		// save result
		if (adc_value[adc_current] > 0) adc_value[adc_current] = (adc_value[adc_current] + ADCH)/2;
		else adc_value[adc_current] = ADCH;
		// round robin: switch to next enabled pin
		if (adc_current < 15) {
			adc_current++;
		} else {
			adc_current = 0;
		}
	} else {
		// trigger ADC conversion
		if (adc_enable[adc_current]) {
			// select ADC pin
			if (adc_current < 8) {
				ADMUX ^= adc_current;
				ADCSRB ^= 0b00000000;
			} else {
				ADMUX ^= adc_current-8;
				ADCSRB ^= 0b00001000;
			}
			// start ADC conversion
			ADCSRA = ADCSRA | (1 << ADSC);
		}
	}
}


// PULSING -----------------------------------------------------------------------------------------------

uint16_t pin_pulse_detect(pin_t pin) {
	bool s0 = read(pin);
	while (read(pin) == s0);
	uint16_t t0 = micros();
	while (read(pin) != s0);
	return (micros() - t0);
}

// note: interrupts are disabled while generating the pulse.
void pin_pulse_single(pin_t pin, uint16_t width) {
	if (width == 0) return;
	uint16_t count = ((width * (F_CPU / 1000000L)) / 4);
	uint8_t sreg = SREG;
	__asm__ __volatile__("cli" ::: "memory");
	SFR(pin)->pins |= GPIO_MASK(pin);
	_delay_loop_2(count);
	SFR(pin)->pins |= GPIO_MASK(pin);
	SREG = sreg;
	__asm__ __volatile__("" ::: "memory");
}

void pin_pulse_multi(uint16_t milli, uint8_t *data) {
	if (pulsing.head) {
		pulsing.tail->next = malloc(sizeof(pin_frame_t));
		pulsing.tail->next->milli = milli;
		// TODO: copy data to frame
		pulsing.tail->next->pulse = frame;
		pulsing.tail->next->next = NULL;
		pulsing.tail = pulsing.tail->next;
	} else {
		pulsing.head = malloc(sizeof(pin_frame_t));
		pulsing.head->milli = milli;
		// TODO: copy data to frame
		pulsing.head->pulse = frame;
		pulsing.head->next = NULL;
		pulsing.tail = pulsing.head;
	}
}


// COMMS -----------------------------------------------------------------------------------

/* UART */

// UART allowed speeds:
// - 2400, error free
// - 4800, error < 0.5%
// - 9600, error < 0.5%
// - 19200, error < 0.5%
// - 38400, error < 0.5%
// - 76800, error < 0.5%
// - 250000, error free
void uart0_enable(uint32_t baud = 0) {
	// defaults to 250000bps
	uint32_t baudrate = 3;
	if (baud != 0) {
		baudrate = (F_CPU/(baud*16UL))-1;
	}
	// set baudrate
	UBRR0L = baudrate;
	UBRR0H = (baudrate>>8);
	// clear the USART status register
	UCSR0A = 0x00;
	// enable rx, tx and rx interrupt
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	// async-mode 	
	UCSR0C = (1<<URSEL)|(1<<UCSZ01)|(1<<UCSZ00);
}

uint8_t uart0_need_rx(void) {
	return cbuf_is_empty(&uart0_commport.rx_buf);
}

uint8_t uart0_rx_byte(unsigned char *data) {
	if (uart0_rx_error > 0) {
		// TODO
		return 0;
	}
	uart0_rx_error = UART_ERROR_NO;
	// TODO: move 1 byte from buffer to *data
}

uint8_t uart0_rx_all(unsigned char *data) {
	if (uart0_rx_error > 0) {
		// TODO
		return 0;
	}
	uart0_rx_error = UART_ERROR_NO;
	// TODO: move all buffer to *data
}

uint8_t uart0_need_tx(void) {
	return cbuf_is_empty(&uart0_commport.tx_buf);
}

void uart0_tx_byte(unsigned char data) {
}

void uart0_tx_all(unsigned char data) {
}

void uart0_disable(void) {
	UCSR0B = (0<<RXEN0)|(0<<TXEN0)|(0<<RXCIE0);
}

ISR(USART0_RX_vect) {
	unsigned char data;
	unsigned char tmphead;
	data = UDR0; // read the received data
	tmphead = ( uart_rx_head + 1 ) & UART_RX_BUFFER_MASK; // calculate buffer index
	uart_rx_head = tmphead; // store new index
	if ( tmphead == uart_rx_tail )  {
		uart0_rx_error = UART_ERROR_BUFOVF;
	}
	uart_rx_buf[tmphead] = data; // store received data in buffer
}

ISR(USART0_UDRE_vect) {
	unsigned char tmptail;
	// check if all data is transmitted
	if ( uart0_tx_head != uart0_tx_tail ) {
		// calculate buffer index
		tmptail = ( uart0_tx_tail + 1 ) & UART_TX_BUFFER_MASK;
		uart0_tx_tail = tmptail; // store new index
		UDR0 = uart0_tx_buf[tmptail]; // start transmition
	} else {
		UCSR0B &= ~(1<<UDRIE0); // disable UDRE interrupt
	}
}


/* I2C */

void i2c_enable(void) {}
uint8_t i2c_need_rx(void) {}
uint8_t i2c_rx_byte(void) {}
uint8_t i2c_rx_all(void) {}
uint8_t i2c_need_tx(void) {}
uint8_t i2c_tx_byte(void) {}
uint8_t i2c_tx_all(void) {}
void i2c_disable(void) {}

ISR(TWI_vect) {
}


/* SPI */

// SPI slave
void spi_enable(void) {
	DDRB = (1<<PINB4); // MISO as OUTPUT
	SPCR = (1<<SPE)|(1<<SPIE); // Enable SPI && interrupt enable bit
	SPDR = 0;
}
uint8_t spi_need_rx(void) {}
uint8_t spi_rx_byte(void) {}
uint8_t spi_rx_all(void) {}
uint8_t spi_need_tx(void) {}
uint8_t spi_tx_byte(void) {}
uint8_t spi_tx_all(void) {}
void spi_disable(void) {}

ISR(SPI_STC_vect) {
	uint8_t y = SPDR;
	PORTD = y;
}


// ARCH -----------------------------------------------------------------------------------

void logger(const char *string) {}

void arch_init() {
	// default text console
	uart_enable();

	// pulsing pins
	pulsing_enable();

	// Timer2: pulse trigger, the counter starts at zero and resets to zero at TOP (compA value).
	TCCR2 |= (1<<CS21); // Fast PWM mode, with prescaler = 1, ie: 1 tick each 62.5ns
	TCNT2 = 0; // initialize counter
	OCR2A = 63; // compA, TOP (default 4us)
	OCR2B = 255; // compB, MATCH (255>TOP -> never matches = no pulse)
	TIMSK2 |= (1<<OCIE2A)|(1<<OCIE2B)|(1<<TOIE2); // enable interrupts

	// Timer1: time keeping
	TCCR1 |= (1<<CS11); // Fast PWM mode, with prescaler = 1, ie: 1 tick each 62.5ns
	TCNT1 = 0; // initialize counter
	OCR1A = 63; // compA, 4us
	OCR1B = 15999; // compB, 1ms
	TIMSK1 |= (1<<OCIE1A)|(1<<OCIE1B); // enable interrupts

	// Timer0: system tasks
	TCCR0 |= (1<<CS01); // Normal mode, with prescaler = 64, ie: 1 tick each 4us
	TCNT0 = 0; // initialize counter
	OCR0A = 124; // compA, 0.5ms
	OCR0B = 249; // compB, 1ms
	TIMSK0 |= (1<<OCIE0A)|(1<<OCIE0B); // enable interrupts

	// ADC init and first discarded conversion to warm-up circuitry
	ADMUX |= (1 << REFS0); // reference voltage on AVCC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // ADC clock prescaler / 64 (250KHz)
	ADMUX |= (1 << ADLAR); // adjust result -> return 8 bits in ADCH
	ADCSRA |= (1 << ADEN); // enable ADC
	ADCSRA |= (1 << ADSC); // start first conversion
	while(ADCSRA & (1 << ADSC)); // wait for first result
	uint8_t fr = ADCH; // discard first result

	// enable global interrupts
	sei();
}

// Timer2: pulse generator, TOP and MATCH
ISR(TIMER2_COMPA_vect) {
	// TOP
}
ISR(TIMER2_COMPB_vect) {
	// TODO check cycles < 32 (ie: t < 2us)
	// set pins to high
	pin_write(pulsing.pin[0], BIT_GET(pulsing.head->pulse[pulsing_counter], BIT(0)) );
	pin_write(pulsing.pin[1], BIT_GET(pulsing.head->pulse[pulsing_counter], BIT(1)) );
	pin_write(pulsing.pin[2], BIT_GET(pulsing.head->pulse[pulsing_counter], BIT(2)) );
	pin_write(pulsing.pin[3], BIT_GET(pulsing.head->pulse[pulsing_counter], BIT(3)) );
	pin_write(pulsing.pin[4], BIT_GET(pulsing.head->pulse[pulsing_counter], BIT(4)) );
	pin_write(pulsing.pin[5], BIT_GET(pulsing.head->pulse[pulsing_counter], BIT(5)) );
	pin_write(pulsing.pin[6], BIT_GET(pulsing.head->pulse[pulsing_counter], BIT(6)) );
	pin_write(pulsing.pin[7], BIT_GET(pulsing.head->pulse[pulsing_counter], BIT(7)) );
}
ISR(TIMER2_OVF_vect) {
	// TODO check cycles < 32 (ie: t < 2us)
	// set pins to low
	pin_write(pulsing.pin[0], 0);
	pin_write(pulsing.pin[1], 0);
	pin_write(pulsing.pin[2], 0);
	pin_write(pulsing.pin[3], 0);
	pin_write(pulsing.pin[4], 0);
	pin_write(pulsing.pin[5], 0);
	pin_write(pulsing.pin[6], 0);
	pin_write(pulsing.pin[7], 0);
}

// Timer1
ISR(TIMER1_COMPA_vect) {
	// update clock
	milli++;
	// reset pulse counter
	pulsing_counter = 0;
	OCR1B = 63;
	// signal overflow
	t1_ovf = 1;
}
ISR(TIMER1_COMPB_vect) {
	pulsing_counter++;
	OCR1B = OCR1B + 64;
}

// Timer0
ISR(TIMER0_COMPA_vect) {
	// update clock
	if (milli>=999) {
		milli = milli - 999;
		timestamp++;
	}
}
ISR(TIMER0_COMPB_vect) {
	// discard current frame
	if (pulsing.head->next) {
		pin_frame_t old = pulsing.head;
		pulsing.head = pulsing.head->next;
		free(old);
	}
	uint16_t elapsed = cycles_end();
	// re-start cycles count for the next 1ms
	cycles_start();
	// percent of cpu usage
	lag = elapsed/160; // used cpu power, % points
}

uint8_t arch_run(uint16_t snow, uint16_t mnow) {
	if (lag >= 100) {
		// TODO error: signal lag
	else {
		// run ADC
		pin_input_adc_run();
	}
	uint8_t cpu_usage = lag;
	lag = 0;
	return cpu_usage;
}

