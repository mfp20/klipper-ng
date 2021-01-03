#ifndef __AVR_WATCHDOG_H
#define __AVR_WATCHDOG_H

#include <stdint.h> // uint32_t

void task_init_watchdog(void);
void task_watchdog(void);

void command_reset(uint32_t*);

#endif // watchdog.h
