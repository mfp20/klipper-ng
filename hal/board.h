#ifndef PLATFORM_BOARD_H
#define PLATFORM_BOARD_H

#include "arch.h"

void board_init(void);
uint16_t board_run(uint16_t snow, uint16_t mnow, uint16_t utimeout);

#endif

