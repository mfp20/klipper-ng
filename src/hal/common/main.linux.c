// Main starting point for micro-controller code running on linux systems
//
// Copyright (C) 2017  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include </usr/include/sched.h> // sched_setscheduler
#include <stdio.h> // fprintf
#include <string.h> // memset
#include <unistd.h> // getopt

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
	initFunc[1] = task_init_timer;

	// task functions
	taskFunc[0] = task_analog_in;
#ifdef CONFIG_ASDASDDASDASDA
	taskFunc[1] = task_thermocouple;
#else
	taskFunc[1] = task_noop;
#endif
	taskFunc[2] = timespec_update;
	taskFunc[3] = task_console_rx;
	taskFunc[4] = task_watchdog;

	// end functions
	endFunc[0] = task_end_tx;
	endFunc[1] = task_end_move;
	endFunc[2] = task_end_analog_in;
	endFunc[3] = task_end_spidev;
	endFunc[4] = task_end_pwm;

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


/****************************************************************
 * Real-time setup
 ****************************************************************/

static int realtime_setup(void) {
    struct sched_param sp;
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = 1;
    int ret = sched_setscheduler(0, SCHED_FIFO, &sp);
    if (ret < 0) {
        report_errno("sched_setscheduler", ret);
        return -1;
    }
    return 0;
}


/****************************************************************
 * Restart
 ****************************************************************/

static char **orig_argv;

void command_config_reset(uint32_t *args) {
    if (! sched_is_shutdown())
        sched_shutdown(ERROR_NOT_SHUTDOWN);
    int ret = execv(orig_argv[0], orig_argv);
    report_errno("execv", ret);
}


/****************************************************************
 * Startup
 ****************************************************************/

int main(int argc, char **argv) {
    // Parse program args
    orig_argv = argv;
    int opt, watchdog = 0, realtime = 0;
    while ((opt = getopt(argc, argv, "wr")) != -1) {
        switch (opt) {
        case 'w':
            watchdog = 1;
            break;
        case 'r':
            realtime = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-w] [-r]\n", argv[0]);
            return -1;
        }
    }

    // Initial setup
    if (realtime) {
        int ret = realtime_setup();
        if (ret)
            return ret;
    }
    int ret = console_setup("/tmp/klipper_host_mcu");
    if (ret)
        return -1;
    if (watchdog) {
        int ret = watchdog_setup();
        if (ret)
            return ret;
    }

    // Main loop
    sched_main();
    return 0;
}

