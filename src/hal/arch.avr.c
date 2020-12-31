
#include <hal/arch.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stddef.h>

volatile uint8_t pin[PIN_TOTAL];
volatile uint8_t pin_adc_enable[16];
volatile uint8_t pin_adc_value[16];
volatile uint8_t pin_adc_current = 0;
volatile pin_pulsing_t pin_pin_pulsing;

volatile uint16_t tnano = 0;
volatile uint16_t tmicro = 0;
volatile uint16_t tmilli = 0;
volatile uint16_t tsecond = 0;

// AVR Costs:
// - reg/int8 assignment/increment, 1 cycle
// - ISR, 4 cycles to enter the ISR and another 4 for reti
// - function call, 5 cycles
// ...

// General Purpose Digital I/O Control Registers.
typedef struct gpio_reg_s {
	volatile uint8_t pins;	//!< Port Input Pins.
	volatile uint8_t ddr;	//!< Data Direction Register.
	volatile uint8_t port;	//!< Data Register.
} gpio_reg_t;

// Return pointer to control registers.
__attribute__((always_inline))
gpio_reg_t* SFR(uint8_t pin) {
	return ((gpio_reg_t*) GPIO_REG(pin));
}

__attribute__((always_inline))
void pin_input_enable(uint8_t pin) {
	GPIO_ATOMIC(pin, SFR(pin)->ddr &= ~GPIO_MASK(pin));
}

__attribute__((always_inline))
void pin_output_enable(uint8_t pin) {
	GPIO_ATOMIC(pin, SFR(pin)->ddr |= GPIO_MASK(pin));
}

__attribute__((always_inline))
uint8_t pin_read(uint8_t pin, uint8_t timeout) {
    return ((SFR(pin)->pins & GPIO_MASK(pin)) != 0);
}

__attribute__((always_inline))
void pin_set_low(uint8_t pin) {
	GPIO_ATOMIC(pin, SFR(pin)->port &= ~GPIO_MASK(pin));
}

__attribute__((always_inline))
void pin_set_high(uint8_t pin) {
	GPIO_ATOMIC(pin, SFR(pin)->port |= GPIO_MASK(pin));
}

__attribute__((always_inline))
void uint8_toggle(uint8_t pin) {
	GPIO_ATOMIC(pin, SFR(pin)->pins |= GPIO_MASK(pin));
}

__attribute__((always_inline))
void pin_write(uint8_t pin, uint8_t value) {
	if (pin) if (value) pin_set_high(pin); else pin_set_low(pin);
}

__attribute__((always_inline))
void pin_pullup_enable(uint8_t pin) {
	pin_set_high(pin);
}

__attribute__((always_inline))
void pin_open_drain(uint8_t pin) {
    pin_input_enable(pin);
    pin_set_low(pin);
}


// TIME -----------------------------------------------------------------------------------

// cpu cycles counter
volatile uint16_t cycle = 0;
// overflow flags
volatile uint8_t t1_ovf = 0;
volatile uint8_t cycle_ovf = 0;

__attribute__((always_inline))
static void cycles_start(void) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		cycle = (uint16_t)TCNT1;
		cycle_ovf = t1_ovf;
	}
}
__attribute__((always_inline))
static uint16_t cycles_report(void) {
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
__attribute__((always_inline))
static uint16_t cycles_end(void) {
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

__attribute__((always_inline))
uint16_t nanos(void) {
	uint16_t ticks;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		ticks = (uint16_t)TCNT1;
	}
	return (ticks*62.5);
}

__attribute__((always_inline))
uint16_t micros(void) {
	uint16_t ticks;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		ticks = (uint16_t)TCNT1;
	}
	return ((ticks*62.5)/1000);
}

__attribute__((always_inline))
uint16_t millis(void) {
	uint16_t ms;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if (tmilli>=999) {
			tmilli = tmilli - 999;
			tsecond++;
		}
		ms = tmilli;
	}
	return ms;
}

//
__attribute__((always_inline))
uint16_t seconds(void) {
	uint16_t ticks;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		if (tmilli>=999) {
			tmilli = tmilli - 999;
			tsecond++;
		}
		ticks = tsecond;
	}
	return ticks;
}


// ADC -----------------------------------------------------------------------------------------------

static void pin_input_adc_run(void) {
	// ADC, if the ADSC bit has been cleared, result is ready
	if(ADCSRA & (0 << ADSC)) {
		// save result
		if (pin_adc_value[pin_adc_current] > 0)
			pin_adc_value[pin_adc_current] = (pin_adc_value[pin_adc_current] + ADCH)/2;
		else pin_adc_value[pin_adc_current] = ADCH;
		// round robin: switch to next enabled pin
		if (pin_adc_current < 15) {
			pin_adc_current++;
		} else {
			pin_adc_current = 0;
		}
	} else {
		// trigger ADC conversion
		if (pin_adc_enable[pin_adc_current]) {
			// select ADC pin
			if (pin_adc_current < 8) {
				ADMUX ^= pin_adc_current;
				ADCSRB ^= 0b00000000;
			} else {
				ADMUX ^= pin_adc_current-8;
				ADCSRB ^= 0b00001000;
			}
			// start ADC conversion
			ADCSRA = ADCSRA | (1 << ADSC);
		}
	}
}


// PULSING -----------------------------------------------------------------------------------------------

uint16_t pin_pulse_detect(uint8_t pin) {
	uint8_t s0 = read(pin);
	while (read(pin) == s0);
	uint16_t t0 = micros();
	while (read(pin) != s0);
	return (micros() - t0);
}

// note: interrupts are disabled while generating the pulse.
void pin_pulse_single(uint8_t pin, uint16_t width) {
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

volatile pin_frame_t* pin_pulse_multi(uint16_t milli) {
	if (pin_pulsing.head) {
		pin_pulsing.tail->next = malloc(sizeof(pin_frame_t));
		pin_pulsing.tail->next->milli = milli;
		// TODO: copy data to frame
		pin_pulsing.tail->next->pulse = frame;
		pin_pulsing.tail->next->next = NULL;
		pin_pulsing.tail = pin_pulsing.tail->next;
	} else {
		pin_pulsing.head = malloc(sizeof(pin_frame_t));
		pin_pulsing.head->milli = milli;
		// TODO: copy data to frame
		pin_pulsing.head->pulse = frame;
		pin_pulsing.head->next = NULL;
		pin_pulsing.tail = pin_pulsing.head;
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
// - 500000, error free
// - 1000000, error free
// - 2000000, error free
void uart0_enable(uint32_t baud) {
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
	// enable rx, tx and rx-complete interrupt
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
	uart0_rx_error = COMMPORT_ERROR_NO;
	// TODO: move 1 byte from buffer to *data
}

uint8_t uart0_rx_all(unsigned char *data) {
	if (uart0_rx_error > 0) {
		// TODO
		return 0;
	}
	uart0_rx_error = COMMPORT_ERROR_NO;
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

// rx action
ISR(USART0_RX_vect) {
	unsigned char data;
	unsigned char tmphead;
	data = UDR0; // read the received data
	tmphead = ( uart_rx_head + 1 ) & RX_BUFFER_MASK; // calculate buffer index
	uart_rx_head = tmphead; // store new index
	if ( tmphead == uart_rx_tail )  {
		uart0_rx_error = COMMPORT_ERROR_BUFOVF;
	}
	uart_rx_buf[tmphead] = data; // store received data in buffer
}

ISR(USART0_UDRE_vect) {
	unsigned char tmptail;
	// check if all data is transmitted
	if ( uart0_tx_head != uart0_tx_tail ) {
		// calculate buffer index
		tmptail = ( uart0_tx_tail + 1 ) & TX_BUFFER_MASK;
		uart0_tx_tail = tmptail; // store new index
		UDR0 = uart0_tx_buf[tmptail]; // start transmition
	} else {
		UCSR0B &= ~(1<<UDRIE0); // disable UDRE interrupt
	}
}


/* I2C */

ISR(TWI_vect) {
}


/* SPI */

// SPI slave
void spi_enable(void) {
	DDRB = (1<<PINB4); // MISO as OUTPUT
	SPCR = (1<<SPE)|(1<<SPIE); // Enable SPI && interrupt enable bit
	SPDR = 0;
}

ISR(SPI_STC_vect) {
	uint8_t y = SPDR;
	PORTD = y;
}


// ARCH -----------------------------------------------------------------------------------

void logger(const char *string) {}

void arch_init() {
	// default text console
	uart_enable();

	// pin_pulsing.pins
	pin_pulsing.enable();

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

// Timer2: pulse generator
// example: COMPA = 64, COMPB = 32
// Pins are LOW. The counter starts counting from 0; at 32 it matches COMPB, selected pins become HIGH.
// The counter keeps counting from 33; at 64 it maches COMPA, triggers OVF and restart from 0. OVF set selected pins to LOW.
// Pins are LOW from 0 to 32, HIGH from 33 to 64. Then low again.
ISR(TIMER2_COMPA_vect) {
	// TOP: once reached, counter goes back to bottom
}
ISR(TIMER2_COMPB_vect) {
	// MATCH: once reached, set pins to high; ie: set match < top, to enable a pulse between match and top
	pin_write(pin_pulsing.pin[0], BIT_GET(pin_pulsing.head->pulse[pin_pulsing.counter], BIT(0)) );
	pin_write(pin_pulsing.pin[1], BIT_GET(pin_pulsing.head->pulse[pin_pulsing.counter], BIT(1)) );
	pin_write(pin_pulsing.pin[2], BIT_GET(pin_pulsing.head->pulse[pin_pulsing.counter], BIT(2)) );
	pin_write(pin_pulsing.pin[3], BIT_GET(pin_pulsing.head->pulse[pin_pulsing.counter], BIT(3)) );
	pin_write(pin_pulsing.pin[4], BIT_GET(pin_pulsing.head->pulse[pin_pulsing.counter], BIT(4)) );
	pin_write(pin_pulsing.pin[5], BIT_GET(pin_pulsing.head->pulse[pin_pulsing.counter], BIT(5)) );
	pin_write(pin_pulsing.pin[6], BIT_GET(pin_pulsing.head->pulse[pin_pulsing.counter], BIT(6)) );
	pin_write(pin_pulsing.pin[7], BIT_GET(pin_pulsing.head->pulse[pin_pulsing.counter], BIT(7)) );
}
ISR(TIMER2_OVF_vect) {
	// overflow: every time counter reaches TOP, set pins to low
	pin_write(pin_pulsing.pin[0], 0);
	pin_write(pin_pulsing.pin[1], 0);
	pin_write(pin_pulsing.pin[2], 0);
	pin_write(pin_pulsing.pin[3], 0);
	pin_write(pin_pulsing.pin[4], 0);
	pin_write(pin_pulsing.pin[5], 0);
	pin_write(pin_pulsing.pin[6], 0);
	pin_write(pin_pulsing.pin[7], 0);
}

// Timer1
ISR(TIMER1_COMPA_vect) {
	// update clock
	tmilli++;
	// reset pulse counter
	pin_pulsing.counter = 0;
	OCR1B = 63;
	// signal overflow
	t1_ovf = 1;
}
ISR(TIMER1_COMPB_vect) {
	pin_pulsing.counter++;
	OCR1B = OCR1B + 64;
}

// Timer0
ISR(TIMER0_COMPA_vect) {
	// update clock
	if (tmilli>=999) {
		tmilli = tmilli - 999;
		tsecond++;
	}
}
ISR(TIMER0_COMPB_vect) {
	// discard current frame
	if (pin_pulsing.head->next) {
		pin_frame_t old = pin_pulsing.head;
		pin_pulsing.head = pin_pulsing.head->next;
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
	} else {
		// run ADC
		pin_input_adc_run();
	}
	uint8_t cpu_usage = lag;
	lag = 0;
	return cpu_usage;
}

