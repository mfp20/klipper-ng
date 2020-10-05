
#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#include <hal/arch.h>

void board_init(void);
uint16_t board_run(uint16_t snow, uint16_t mnow, uint16_t utimeout);

#endif

