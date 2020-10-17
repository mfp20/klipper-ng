
#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#include <hal/arch.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h> // for uint8_t

// init board (NOTE: not MCU, see arch_init() for MCU init)
void _board_init(void);
void board_init(void);

// tasks to run every loop cycle
void _board_run(void);
void board_run(void);

void _board_reset(void);
uint8_t board_reset(void);

#ifdef __cplusplus
}
#endif

#endif

