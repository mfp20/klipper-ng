// Code for parsing incoming commands and encoding outgoing messages
//
// Copyright (C) 2016,2017  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdarg.h> // va_start
#include <string.h> // memcpy
//
#include "cmd.h" // output_P
//
#include "protocol.h" // TYPE_*
#include "avr.h"
#include "serial.h"
#include "console.h"
#include "watchdog.h"
#include "initial_pins.h"
#include "rxtx_irq.h"
#include "cmds_base.h"
#include "cmds_debug.h"
#include "cmds_gpio.h"
#include "cmds_pwm.h"
#include "cmds_adc.h"
#include "cmds_i2c.h"
#include "cmds_spi.h"
#include "spi_software.h"
//#include "driver/stepper.h"
//#include "driver/lcd_st7920.h"
//#include "driver/buttons.h"
//#include "driver/neopixel.h"
//#include "driver/endstop.h"
//#include "driver/thermocouple.h"
//#include "driver/lcd_hd44780.h"
//#include "driver/tmcuart.h"
//
#include "sched.h" // sched_is_shutdown
#include "common_io.h" // readb
//
#include "avr.h" // crc16_ccitt
#include "irq.h" // irq_poll
#include "pgm.h" // READP
#include "timer.h" //


/****************************************************************
 * task handlers
 ****************************************************************/

void task_noop(void) {}


/****************************************************************
 * command handlers
 ****************************************************************/

uint8_t *command_unknown(uint8_t *start, uint8_t *end) {
	// TODO
	return end;
}
uint8_t *command_noop(uint8_t *start, uint8_t *end) {return end;}

/****************************************************************
 * function pointer arrays
 ****************************************************************/

voidPtr initFunc[FUNCS_INIT_NO];
voidPtr taskFunc[FUNCS_TASK_NO];
voidPtr endFunc[FUNCS_END_NO];
cmdPtr cmdFunc[TYPE_NO];

void funcs_init(void) {
	// init functions
	initFunc[0] = alloc_init;
	initFunc[1] = initial_pins_setup;
	initFunc[2] = prescaler_init;
	initFunc[3] = timer_init;
	initFunc[4] = watchdog_init;
	initFunc[5] = serial_init;

	// task functions
#ifdef CONFIG_ASDASDDASDASDA // TODO
	taskFunc[0] = endstop_task;
#else
	taskFunc[0] = task_noop;
#endif
	taskFunc[1] = analog_in_task;
#ifdef CONFIG_ASDASDDASDASDA
	taskFunc[2] = thermocouple_task;
#else
	taskFunc[2] = task_noop;
#endif
#ifdef CONFIG_ASDASDDASDASDA
	taskFunc[3] = buttons_task;
#else
	taskFunc[3] = task_noop;
#endif
#ifdef CONFIG_ASDASDDASDASDA
	taskFunc[4] = tmcuart_task;
#else
	taskFunc[4] = task_noop;
#endif
	taskFunc[5] = watchdog_reset;
	taskFunc[6] = console_rx;

	// end functions
	endFunc[0] = tx_end;
	endFunc[1] = move_reset;
	endFunc[2] = soft_pwm_shutdown;
	endFunc[3] = digital_out_shutdown;
#ifdef CONFIG_ASDASDDASDASDA
	endFunc[4] = stepper_shutdown;
#else
	endFunc[4] = task_noop;
#endif
	endFunc[5] = analog_in_shutdown;
	endFunc[6] = spidev_shutdown;
	endFunc[7] = pwm_shutdown;
#ifdef CONFIG_ASDASDDASDASDA
	endFunc[8] = st7920_shutdown;
#else
	endFunc[8] = task_noop;
#endif
#ifdef CONFIG_ASDASDDASDASDA
	endFunc[9] = hd44780_shutdown;
#else
	endFunc[9] = task_noop;
#endif
#ifdef CONFIG_ASDASDDASDASDA
	endFunc[10] = tmcuart_shutdown;
#else
	endFunc[10] = task_noop;
#endif
	endFunc[11] = timer_reset;

	// command functions TODO
	cmdFunc[TYPE_BASE_IDENTIFY] = command_unknown;
	cmdFunc[TYPE_BASE_ALLOCATE_OIDS] = command_unknown;
	cmdFunc[TYPE_BASE_GET_CONFIG] = command_unknown;
	cmdFunc[TYPE_BASE_FINALIZE_CONFIG] = command_unknown;
	cmdFunc[TYPE_BASE_GET_CLOCK] = command_unknown;
	cmdFunc[TYPE_BASE_GET_UPTIME] = command_unknown;
	cmdFunc[TYPE_BASE_EMERGENCY_STOP] = command_unknown;
	cmdFunc[TYPE_BASE_CLEAR_SHUTDOWN] = command_unknown;
	cmdFunc[TYPE_BASE_RESET] = command_unknown;
	cmdFunc[TYPE_DEBUG_START_GROUP] = command_unknown;
	cmdFunc[TYPE_DEBUG_END_GROUP] = command_unknown;
	cmdFunc[TYPE_DEBUG_NOP] = command_unknown;
	cmdFunc[TYPE_DEBUG_PING] = command_unknown;
	cmdFunc[TYPE_DEBUG_READ] = command_unknown;
	cmdFunc[TYPE_DEBUG_WRITE] = command_unknown;
	cmdFunc[TYPE_GPIO_CONFIG] = command_unknown;
	cmdFunc[TYPE_GPIO_SCHEDULE] = command_unknown;
	cmdFunc[TYPE_GPIO_UPDATE] = command_unknown;
	cmdFunc[TYPE_GPIO_SET] = command_unknown;
	cmdFunc[TYPE_GPIO_SOFTPWM_CONFIG] = command_unknown;
	cmdFunc[TYPE_GPIO_SOFTPWM_SCHEDULE] = command_unknown;
	cmdFunc[TYPE_PWM_CONFIG] = command_unknown;
	cmdFunc[TYPE_PWM_SCHEDULE] = command_unknown;
	cmdFunc[TYPE_PWM_SET] = command_unknown;
	cmdFunc[TYPE_ADC_CONFIG] = command_unknown;
	cmdFunc[TYPE_ADC_QUERY] = command_unknown;
	cmdFunc[TYPE_I2C_CONFIG] = command_unknown;
	cmdFunc[TYPE_I2C_MODBITS] = command_unknown;
	cmdFunc[TYPE_I2C_READ] = command_unknown;
	cmdFunc[TYPE_I2C_WRITE] = command_unknown;
	cmdFunc[TYPE_SPI_CONFIG] = command_unknown;
	cmdFunc[TYPE_SPI_CONFIG_NOCS] = command_unknown;
	cmdFunc[TYPE_SPI_SET] = command_unknown;
	cmdFunc[TYPE_SPI_TRANSFER] = command_unknown;
	cmdFunc[TYPE_SPI_SEND] = command_unknown;
	cmdFunc[TYPE_SPI_SHUTDOWN] = command_unknown;
}


/****************************************************************
 * command/response encoding/decoding
 ****************************************************************/

// Encode an integer as a variable length quantity (vlq)
static uint8_t *vlq_encode(uint8_t *p, uint32_t v) {
	int32_t sv = v;
	if (sv < (3L<<5)  && sv >= -(1L<<5))  goto f4;
	if (sv < (3L<<12) && sv >= -(1L<<12)) goto f3;
	if (sv < (3L<<19) && sv >= -(1L<<19)) goto f2;
	if (sv < (3L<<26) && sv >= -(1L<<26)) goto f1;
	*p++ = (v>>28) | 0x80;
f1: *p++ = ((v>>21) & 0x7f) | 0x80;
f2: *p++ = ((v>>14) & 0x7f) | 0x80;
f3: *p++ = ((v>>7) & 0x7f) | 0x80;
f4: *p++ = v & 0x7f;
	return p;
}

// Parse an integer that was encoded as a "variable length quantity"
static uint32_t vlq_decode(uint8_t **pp) {
	uint8_t *p = *pp, c = *p++;
	uint32_t v = c & 0x7f;
	if ((c & 0x60) == 0x60)
		v |= -0x20;
	while (c & 0x80) {
		c = *p++;
		v = (v<<7) | (c & 0x7f);
	}
	*pp = p;
	return v;
}

// encode and transmit a "command" message
void send_command(uint8_t typ, ...) {
	if (typ >= TYPE_NO); // TODO error

	uint8_t len = 0;
	uint8_t *buf_start = NULL;
	va_list args;
	va_start(args, typ);
	switch (typ) {
		case TYPE_UNKNOWN:
			break;
		case TYPE_NOOP:
			break;
		case TYPE_START:
			break;
		case TYPE_ACK:
			break;
		case TYPE_NACK:
			break;
		case TYPE_SHUTDOWN_NOW:
			break;
		case TYPE_SHUTDOWN_LAST:
			break;
		case TYPE_BASE_IDENTIFY:
			break;
		case TYPE_BASE_ALLOCATE_OIDS:
			break;
		case TYPE_BASE_GET_CONFIG:
			break;
		case TYPE_BASE_FINALIZE_CONFIG:
			break;
		case TYPE_BASE_GET_CLOCK:
			break;
		case TYPE_BASE_GET_UPTIME:
			break;
		case TYPE_BASE_EMERGENCY_STOP:
			break;
		case TYPE_BASE_CLEAR_SHUTDOWN:
			break;
		case TYPE_BASE_STATS:
			break;
		case TYPE_BASE_RESET:
			break;
		case TYPE_DEBUG_START_GROUP:
			break;
		case TYPE_DEBUG_END_GROUP:
			break;
		case TYPE_DEBUG_NOP:
			break;
		case TYPE_DEBUG_PING:
			break;
		case TYPE_DEBUG_READ:
			break;
		case TYPE_DEBUG_WRITE:
			break;
		case TYPE_GPIO_CONFIG:
			break;
		case TYPE_GPIO_SCHEDULE:
			break;
		case TYPE_GPIO_UPDATE:
			break;
		case TYPE_GPIO_SET:
			break;
		case TYPE_GPIO_SOFTPWM_CONFIG:
			break;
		case TYPE_GPIO_SOFTPWM_SCHEDULE:
			break;
		case TYPE_PWM_CONFIG:
			break;
		case TYPE_PWM_SCHEDULE:
			break;
		case TYPE_PWM_SET:
			break;
		case TYPE_ADC_CONFIG:
			break;
		case TYPE_ADC_QUERY:
			break;
		case TYPE_ADC_STATE:
			break;
		case TYPE_I2C_CONFIG:
			break;
		case TYPE_I2C_MODBITS:
			break;
		case TYPE_I2C_READ:
			break;
		case TYPE_I2C_WRITE:
			break;
		case TYPE_SPI_CONFIG:
			break;
		case TYPE_SPI_CONFIG_NOCS:
			break;
		case TYPE_SPI_SET:
			break;
		case TYPE_SPI_TRANSFER:
			break;
		case TYPE_SPI_SEND:
			break;
		case TYPE_SPI_SHUTDOWN:
			break;
		default:
			// TODO error, not implemented
			break;
	}
	va_end(args);
}

// encode and transmit a "response" message
void send_response(uint8_t typ, ...) {
	if (typ >= TYPE_NO); // TODO error

	uint8_t len = 0;
	uint8_t *buf_start = NULL;
	va_list args;
	va_start(args, typ);
	switch (typ) {
		case TYPE_UNKNOWN:
			break;
		case TYPE_NOOP:
			break;
		case TYPE_START:
			break;
		case TYPE_ACK:
		case TYPE_NACK:
			// - An “ack” is a message block with empty content (ie, a 5 byte message block)
			// and a sequence number greater than the last received host sequence number.
			// - A “nak” is a message block with empty content and a sequence number less than the last received host sequence number.
			tx_buffer_trigger(tx_buffer_alloc(0), 0);
			break;
		case TYPE_SHUTDOWN_NOW:
			break;
		case TYPE_SHUTDOWN_LAST:
			break;
		case TYPE_BASE_IDENTIFY:
			// TODO
			len = 0;
			buf_start = tx_buffer_alloc(1);
			*(buf_start+MSG_POS_PAYLOAD) = 123;
			len++;
			tx_buffer_trigger(buf_start, len);
			break;
		case TYPE_BASE_ALLOCATE_OIDS:
			break;
		case TYPE_BASE_GET_CONFIG:
			break;
		case TYPE_BASE_FINALIZE_CONFIG:
			break;
		case TYPE_BASE_GET_CLOCK:
			break;
		case TYPE_BASE_GET_UPTIME:
			break;
		case TYPE_BASE_EMERGENCY_STOP:
			break;
		case TYPE_BASE_CLEAR_SHUTDOWN:
			break;
		case TYPE_BASE_STATS:
			break;
		case TYPE_BASE_RESET:
			break;
		case TYPE_DEBUG_START_GROUP:
			break;
		case TYPE_DEBUG_END_GROUP:
			break;
		case TYPE_DEBUG_NOP:
			break;
		case TYPE_DEBUG_PING:
			break;
		case TYPE_DEBUG_READ:
			break;
		case TYPE_DEBUG_WRITE:
			break;
		case TYPE_GPIO_CONFIG:
			break;
		case TYPE_GPIO_SCHEDULE:
			break;
		case TYPE_GPIO_UPDATE:
			break;
		case TYPE_GPIO_SET:
			break;
		case TYPE_GPIO_SOFTPWM_CONFIG:
			break;
		case TYPE_GPIO_SOFTPWM_SCHEDULE:
			break;
		case TYPE_PWM_CONFIG:
			break;
		case TYPE_PWM_SCHEDULE:
			break;
		case TYPE_PWM_SET:
			break;
		case TYPE_ADC_CONFIG:
			break;
		case TYPE_ADC_QUERY:
			break;
		case TYPE_ADC_STATE:
			break;
		case TYPE_I2C_CONFIG:
			break;
		case TYPE_I2C_MODBITS:
			break;
		case TYPE_I2C_READ:
			break;
		case TYPE_I2C_WRITE:
			break;
		case TYPE_SPI_CONFIG:
			break;
		case TYPE_SPI_CONFIG_NOCS:
			break;
		case TYPE_SPI_SET:
			break;
		case TYPE_SPI_TRANSFER:
			break;
		case TYPE_SPI_SEND:
			break;
		case TYPE_SPI_SHUTDOWN:
			break;
		default:
			// TODO error, not implemented
			break;
	}
	va_end(args);
}

