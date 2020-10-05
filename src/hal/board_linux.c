
#include <hal/board.h>

void board_init(void) {
	arch_init();
	// init commports and use uart0 as console
	commports[0] = (commport_t*)malloc(sizeof(commport_t)*COMMPORTS_NO);
	commports[0]->fd = uart_enable(commports[0], 0);
	cbuf_init(&commports[0]->rx_buf, RX_BUFFER_SIZE);
	cbuf_init(&commports[0]->tx_buf, TX_BUFFER_SIZE);
	commports[0]->need_rx = uart_need_rx;
	commports[0]->rx_byte = uart_rx_byte;
	commports[0]->rx_all = uart_rx_all;
	commports[0]->need_tx = uart_need_tx;
	commports[0]->tx_byte = uart_tx_byte;
	commports[0]->tx_all = uart_tx_all;
}

uint16_t board_run(uint16_t snow, uint16_t mnow, uint16_t utimeout) {
	uint16_t ustart = micros();
	// arch tasks
	arch_run(snow, mnow, utimeout);
	uint16_t uend = micros();
	uint16_t elapsed = 0;
	if (uend>=ustart) {
		elapsed = uend-ustart;
		//printf("start %d end %d elapsed %d (us)\n", ustart, uend, elapsed);
	} else {
		elapsed = ((999-ustart)+uend);
		//printf("start %d end %d elapsed %d (us) (ovf)\n", ustart, uend, elapsed);
	}
	if (elapsed > utimeout) return elapsed;
	else utimeout = utimeout - elapsed;
	// read/write comms
	
	for(int i=0;i<COMMPORTS_NO;i++) {

		if (commports[i]->need_rx(commports[i])) {
			commports[i]->rx_all(commports[i], utimeout);
		}

		if (commports[i]->need_tx(commports[i])) {
			commports[i]->tx_all(commports[i], utimeout);
		}

	}

	//
	usleep(234); // TODO: dummy delay, to be removed
	uend = micros();
	if (uend>=ustart) {
		elapsed = uend-ustart;
	} else {
		elapsed = ((999-ustart)+uend);
	}
	return elapsed;
}

