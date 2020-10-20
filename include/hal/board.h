#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#ifdef __GIT_REVPARSE__
#define RELEASE_BOARD __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <hal/arch.h>
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

