
#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#include <hal/arch.h>

// init board (NOTE: not MCU, see arch_init() for MCU init)
void board_init(void);
// tasks to run every loop cycle
uint16_t board_run(void);

#endif

