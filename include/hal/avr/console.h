#ifndef __AVR_CONSOLE_H
#define __AVR_CONSOLE_H

#include <inttypes.h>

#include "protocol.h" // RX_BUFFER_SIZE
#include "generic_io.h" // readb

extern uint8_t receive_buf[MSG_RX_BUFFER_SIZE], receive_pos;
extern uint8_t transmit_buf[MSG_TX_BUFFER_SIZE], transmit_pos, transmit_max;

void task_console_rx(void);

#endif // console.h

