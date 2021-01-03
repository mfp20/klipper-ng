// Main starting point for AVR boards.
//
// Copyright (C) 2016,2017  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "initial_pins.h"
#include "rxtx_irq.h"
#include "sched.h"
#include "cmd.h"
#include "cmds_base.h"
#include "cmds_debug.h"
#include "cmds_gpio.h"
#include "cmds_pwm.h"
#include "cmds_adc.h"
#include "cmds_i2c.h"
#include "cmds_spi.h"
#include "cmds_bitbanging.h"


/****************************************************************
 * function pointer arrays
 ****************************************************************/

void funcs_init(void) {
	// init functions
	initFunc[0] = task_init_alloc;
	initFunc[1] = task_init_initial_pins;
#ifdef __BOARD_AVR__
	initFunc[2] = task_init_prescaler;
#else
	initFunc[2] = task_noop;
#endif
	initFunc[3] = task_init_timer;
	initFunc[4] = task_init_watchdog;
	initFunc[5] = task_init_serial;

	// task functions
#ifdef CONFIG_ASDASDDASDASDA // TODO
	taskFunc[0] = task_endstop;
#else
	taskFunc[0] = task_noop;
#endif
	taskFunc[1] = task_analog_in;
#ifdef CONFIG_ASDASDDASDASDA
	taskFunc[2] = task_thermocouple;
#else
	taskFunc[2] = task_noop;
#endif
#ifdef CONFIG_ASDASDDASDASDA
	taskFunc[3] = task_buttons;
#else
	taskFunc[3] = task_noop;
#endif
#ifdef CONFIG_ASDASDDASDASDA
	taskFunc[4] = task_tmcuart;
#else
	taskFunc[4] = task_noop;
#endif
	taskFunc[5] = task_watchdog;
	taskFunc[6] = task_console_rx;

	// end functions
	endFunc[0] = task_end_tx;
	endFunc[1] = task_end_move;
	endFunc[2] = task_end_soft_pwm;
	endFunc[3] = task_end_digital_out;
#ifdef CONFIG_ASDASDDASDASDA
	endFunc[4] = task_end_stepper;
#else
	endFunc[4] = task_noop;
#endif
	endFunc[5] = task_end_analog_in;
	endFunc[6] = task_end_spidev;
	endFunc[7] = task_end_pwm;
#ifdef CONFIG_ASDASDDASDASDA
	endFunc[8] = task_end_st7920;
#else
	endFunc[8] = task_noop;
#endif
#ifdef CONFIG_ASDASDDASDASDA
	endFunc[9] = task_end_hd44780;
#else
	endFunc[9] = task_noop;
#endif
#ifdef CONFIG_ASDASDDASDASDA
	endFunc[10] = task_end_tmcuart;
#else
	endFunc[10] = task_noop;
#endif
	endFunc[11] = task_end_timer;

	// command functions TODO
	cmdFunc[TYPE_BASE_IDENTIFY] = command_identify;
	cmdFunc[TYPE_BASE_ALLOCATE_OIDS] = command_allocate_oids;
	cmdFunc[TYPE_BASE_GET_CONFIG] = command_get_config;
	cmdFunc[TYPE_BASE_FINALIZE_CONFIG] = command_finalize_config;
	cmdFunc[TYPE_BASE_GET_CLOCK] = command_get_clock;
	cmdFunc[TYPE_BASE_GET_UPTIME] = command_get_uptime;
	cmdFunc[TYPE_BASE_EMERGENCY_STOP] = command_emergency_stop;
	cmdFunc[TYPE_BASE_CLEAR_SHUTDOWN] = command_clear_shutdown;
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

// Main entry point for avr code.
int main(void) {
    irq_enable();
    sched_main();
    return 0;
}

