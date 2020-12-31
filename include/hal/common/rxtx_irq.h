#ifndef __RXTX_IRQ_H
#define __RXTX_IRQ_H

#include <stdint.h> // uint32_t

#include "protocol.h" // RX_BUFFER_SIZE

extern uint8_t receive_buf[MSG_RX_BUFFER_SIZE], receive_pos;
extern uint8_t transmit_buf[MSG_TX_BUFFER_SIZE], transmit_pos, transmit_max;

void rx_buffer_add(uint_fast8_t data); // called from irq
void rx_buffer_clean(uint_fast8_t len);
int_fast8_t rx_find_block(uint8_t *buf, uint_fast8_t buf_len, uint_fast8_t *pop_count);
void rx_run_cmds(uint8_t *buf, uint_fast8_t msglen);
int_fast8_t rx_find_and_run(uint8_t *buf, uint_fast8_t buf_len, uint_fast8_t *pop_count);

uint8_t *tx_buffer_alloc(uint_fast8_t len);
void tx_buffer_trigger(uint8_t *buf, uint8_t len);
void tx_end(void);
extern void tx_buffer_enable_irq(void); // callback provided by board specific code
int tx_buffer_next(uint8_t *pdata); // called from irq

#endif // rxtx_irq.h

