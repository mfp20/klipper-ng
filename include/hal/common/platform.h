#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "macro_compiler.h"
#include "protocol.h"

#ifdef __BOARD_AVR__
#define FUNCS_INIT_NO 6
#define FUNCS_TASK_NO 7
#define FUNCS_END_NO 12

#include "autoconf.avr.h"
#include "adc.h"
#include "console.h"
#include "crc16_ccitt.h"
#include "dynpool.h"
#include "gpio.h"
#include "i2c.h"
#include "internal.h"
#include "irq.h"
#include "pgm.h"
#include "prescaler.h"
#include "pwm.h"
#include "serial.h"
#include "spi.h"
#include "timer.h"
#include "watchdog.h"
#elif defined __BOARD_LINUX__
#define FUNCS_INIT_NO 2
#define FUNCS_TASK_NO 5
#define FUNCS_END_NO 5

#include "autoconf.linux.h"
#include "adc.h"
#include "console.h"
#include "gpio.h"
#include "i2c.h"
#include "pwm.h"
#include "spi.h"
#include "timer.h"
#include "watchdog.h"
#include "generic_dynpool.h"
#include "generic_irq.h"
#include "generic_pgm.h"
#include "generic_crc16_ccitt.h"
#elif defined __BOARD_SIMULINUX__
#define FUNCS_INIT_NO 2
#define FUNCS_TASK_NO 5
#define FUNCS_END_NO 5

#include "autoconf.simulinux.h"
#endif // CONFIG_MACH_*

#endif // platform.h

