
#include <hal/board.h>

void _board_init(void) {
}
void board_init(void) {
	_board_init();
}

void _board_run(void) {
}
void board_run(void) {
	_board_init();
}

void _board_reset(void) {
}
uint8_t board_reset(void) {
		_board_reset();
		return 0;
}

