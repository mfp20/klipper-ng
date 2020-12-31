#ifndef __WATCHDOG_H
#define __WATCHDOG_H

#include <stdint.h> // uint32_t

void watchdog_init(void);
void watchdog_reset(void);

void command_reset(uint32_t*);

#endif // watchdog.h
