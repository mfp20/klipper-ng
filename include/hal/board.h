
#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#include <hal/arch.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h> // for uint8_t

// init board (NOTE: not MCU, see arch_init() for MCU init)
void _board_init(void);
inline void board_init(void) {
	_board_init();
}

// tasks to run every loop cycle
void _board_run(void);
inline void board_run(void) {
	_board_init();
}

void _board_reset(void);
inline uint8_t board_reset(void) {
		_board_reset();
		return 0;
}

#ifdef __cplusplus
}
#endif

#endif

