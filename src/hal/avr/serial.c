// AVR serial port code.
//
// Copyright (C) 2016-2018  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <avr/interrupt.h> // USART_RX_vect

#include "macro_compiler.h"

#include "autoconf.avr.h" // CONFIG_SERIAL_BAUD
#include "rxtx_irq.h" // rx_buffer_add
#include "sched.h" //
#include "cmd.h" //

// Reserve serial pins
#if CONFIG_SERIAL_PORT == 0
 #if CONFIG_MACH_atmega1280 || CONFIG_MACH_atmega2560
 #else
 #endif
#elif CONFIG_SERIAL_PORT == 1
#elif CONFIG_SERIAL_PORT == 2
#else
#endif

// Helper macros for defining serial port aliases
#define AVR_SERIAL_REG1(prefix, id, suffix) prefix ## id ## suffix
#define AVR_SERIAL_REG(prefix, id, suffix) AVR_SERIAL_REG1(prefix, id, suffix)

// Serial port register aliases
#define UCSRxA AVR_SERIAL_REG(UCSR, CONFIG_SERIAL_PORT, A)
#define UCSRxB AVR_SERIAL_REG(UCSR, CONFIG_SERIAL_PORT, B)
#define UCSRxC AVR_SERIAL_REG(UCSR, CONFIG_SERIAL_PORT, C)
#define UBRRx AVR_SERIAL_REG(UBRR, CONFIG_SERIAL_PORT,)
#define UDRx AVR_SERIAL_REG(UDR, CONFIG_SERIAL_PORT,)
#define UCSZx1 AVR_SERIAL_REG(UCSZ, CONFIG_SERIAL_PORT, 1)
#define UCSZx0 AVR_SERIAL_REG(UCSZ, CONFIG_SERIAL_PORT, 0)
#define U2Xx AVR_SERIAL_REG(U2X, CONFIG_SERIAL_PORT,)
#define RXENx AVR_SERIAL_REG(RXEN, CONFIG_SERIAL_PORT,)
#define TXENx AVR_SERIAL_REG(TXEN, CONFIG_SERIAL_PORT,)
#define RXCIEx AVR_SERIAL_REG(RXCIE, CONFIG_SERIAL_PORT,)
#define UDRIEx AVR_SERIAL_REG(UDRIE, CONFIG_SERIAL_PORT,)

#if defined(USART_RX_vect)
// The atmega168 / atmega328 doesn't have an ID in the irq names
#define USARTx_RX_vect USART_RX_vect
#define USARTx_UDRE_vect USART_UDRE_vect
#else
#define USARTx_RX_vect AVR_SERIAL_REG(USART, CONFIG_SERIAL_PORT, _RX_vect)
#define USARTx_UDRE_vect AVR_SERIAL_REG(USART, CONFIG_SERIAL_PORT, _UDRE_vect)
#endif

void task_init_serial(void) {
    UCSRxA = CONFIG_SERIAL_BAUD_U2X ? (1<<U2Xx) : 0;
    uint32_t cm = CONFIG_SERIAL_BAUD_U2X ? 8 : 16;
    UBRRx = DIV_ROUND_CLOSEST(CONFIG_CLOCK_FREQ, cm * CONFIG_SERIAL_BAUD) - 1UL;
    UCSRxC = (1<<UCSZx1) | (1<<UCSZx0);
    UCSRxB = (1<<RXENx) | (1<<TXENx) | (1<<RXCIEx) | (1<<UDRIEx);
}

// Rx interrupt - data available to be read.
ISR(USARTx_RX_vect) {
    rx_buffer_add(UDRx);
}

// Tx interrupt - data can be written to serial.
ISR(USARTx_UDRE_vect) {
    uint8_t data;
    int ret = tx_buffer_next(&data);
    if (ret)
        UCSRxB &= ~(1<<UDRIEx);
    else
        UDRx = data;
}

// Enable tx interrupts
void tx_buffer_enable_irq(void) {
    UCSRxB |= 1<<UDRIEx;
}
