// Generic interrupt based serial uart helper code
//
// Copyright (C) 2016-2018  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memmove

#include "rxtx_irq.h"

#include "cmd.h" // misc defines
#include "sched.h" // sched_wake_tasks
#include "generic_io.h" // readb/writeb


uint8_t receive_buf[MSG_RX_BUFFER_SIZE], receive_pos;
uint8_t transmit_buf[MSG_TX_BUFFER_SIZE], transmit_pos, transmit_max;

static uint8_t next_sequence = MSG_DEST;

/****************************************************************
 * Incoming message decoding and routing
 ****************************************************************/

// store incoming data in receive buffer (called from irq)
void rx_buffer_add(uint_fast8_t data) {
    if (data == MSG_SYNC)
        sched_wake_tasks();
    if (receive_pos >= sizeof(receive_buf))
        // Serial overflow - ignore it as crc error will force retransmit
        return;
    receive_buf[receive_pos++] = data;
}

// remove from the beginning of receive buffer the given number of bytes
void rx_buffer_clean(uint_fast8_t len) {
    uint_fast8_t copied = 0;
    for (;;) {
        uint_fast8_t rpos = readb(&receive_pos);
        uint_fast8_t needcopy = rpos - len;
        if (needcopy) {
            memmove(&receive_buf[copied], &receive_buf[copied + len], needcopy - copied);
            copied = needcopy;
            sched_wake_tasks();
        }
        irqstatus_t flag = irq_save();
        if (rpos != readb(&receive_pos)) {
            // Raced with irq handler - retry
            irq_restore(flag);
            continue;
        }
        receive_pos = needcopy;
        irq_restore(flag);
        break;
    }
}

enum { CF_NEED_SYNC=1<<0, CF_NEED_VALID=1<<1 };

// Find a block and decode (COBS)
int_fast8_t rx_find_block(uint8_t *buf, uint_fast8_t buf_len, uint_fast8_t *pop_count) {
    static uint8_t sync_state;
    if (buf_len && sync_state & CF_NEED_SYNC) 
		goto need_sync;
    if (buf_len < MSG_MIN) 
		goto need_more_data;
	// find msg len
	uint_fast8_t msglen = 0;
	for(int i = 0;i<=buf_len;i++) {
  		if(buf[i] == MSG_SYNC) {
  			msglen = i+1;
			break;
 		}
	}
	if (msglen == 0) // no MSG_SYNC found
		goto need_more_data;
	if (msglen < MSG_MIN || msglen > MSG_MAX) // MSG_SYNC out of bounduaries
		goto error;
	// decode cobs in place
	size_t read_index  = 0;
	size_t write_index = 0;
	uint8_t code       = 0;
	uint8_t i          = 0;
	while (read_index < msglen) {
		code = buf[read_index];
		if (read_index + code > msglen && code != 1) {
			goto error;
		}
		read_index++;
		for (i = 1; i < code; i++) {
			read_index++;
			write_index++;
		}
		//
		if (code != 0xFF && read_index != msglen) {
			buf[write_index++] = MSG_SYNC;
		}
	}
	buf[0] = 0; // reset COBS code (to allow CRC check)
	// check dest
	uint_fast8_t msgseq = buf[MSG_POS_SEQ];
	if ((msgseq & ~MSG_SEQ_MASK) != MSG_DEST)
		goto error;
	// check crc
	uint16_t msgcrc = ((buf[msglen-MSG_TRAILER_CRC] << 8) | buf[msglen-MSG_TRAILER_CRC+1]); // crc byte
	uint16_t crc = crc16_ccitt(buf, msglen-MSG_TRAILER_SIZE);
	if (crc != msgcrc)
		goto error;
	sync_state &= ~CF_NEED_VALID;
	*pop_count = msglen;
	// check sequence number
	if (msgseq != next_sequence) {
		// lost message - discard messages until it is retransmitted
		goto nak;
	}
	next_sequence = ((msgseq + 1) & MSG_SEQ_MASK) | MSG_DEST;
	return 1;
need_more_data:
	*pop_count = 0;
	return 0;
error:
	if (buf[0] == MSG_SYNC) {
		// Ignore (do not nak) leading SYNC bytes
		*pop_count = 1;
		return -1;
	}
	sync_state |= CF_NEED_SYNC;
need_sync: ;
		   // Discard bytes until next SYNC found
		   uint8_t *next_sync = memchr(buf, MSG_SYNC, buf_len);
		   if (next_sync) {
			   sync_state &= ~CF_NEED_SYNC;
			   *pop_count = next_sync - buf + 1;
		   } else {
			   *pop_count = buf_len;
		   }
		   if (sync_state & CF_NEED_VALID)
			   return -1;
		   sync_state |= CF_NEED_VALID;
nak:
		   send_response(TYPE_NACK);
		   return -1;
}

// run all the commands found in a message block
void rx_run_cmds(uint8_t *buf, uint_fast8_t msglen) {
	uint8_t *start = &buf[MSG_HEADER_SIZE];
	uint8_t *end = &buf[msglen-MSG_TRAILER_SIZE];
	while (start < end) {
		start = cmdFunc[*start](start, end);
		//TODO add flag check: if (sched_is_shutdown() && !(READP(cp->flags) & HF_IN_SHUTDOWN)) {
		if (sched_is_shutdown()) {
			sched_report_shutdown();
			continue;
		}
		irq_poll();
	}
}

// find a message block, decode (COBS), and then run all the commands in it
int_fast8_t rx_find_and_run(uint8_t *buf, uint_fast8_t buf_len, uint_fast8_t *pop_count) {
	int_fast8_t ret = rx_find_block(buf, buf_len, pop_count);
	if (ret > 0) {
		rx_run_cmds(buf, *pop_count);
		send_response(TYPE_ACK);
	}
	return ret;
}

/****************************************************************
 * Outgoing message encoding
 ****************************************************************/

static uint8_t in_tx;

// allocate len bytes in tx buffer
uint8_t *tx_buffer_alloc(uint_fast8_t len) {
    if (readb(&in_tx))
        // This sendf call was made from an irq handler while the main
        // code was already in sendf - just drop this sendf request.
        return NULL;
    writeb(&in_tx, 1); // guard on

	if (len > MSG_PAYLOAD_MAX_SIZE) {
		// message too long
		return NULL;
	}
    uint_fast8_t msglen = len + MSG_MIN;

    // Verify space for message
    uint_fast8_t tpos = readb(&transmit_pos), tmax = readb(&transmit_max);
    if (tpos >= tmax) {
        tpos = tmax = 0;
        writeb(&transmit_max, 0);
        writeb(&transmit_pos, 0);
    }
    if (tmax + msglen > sizeof(transmit_buf)) {
        if (tmax + msglen - tpos > sizeof(transmit_buf))
            // Not enough space for message
            return NULL;
        // Disable TX irq and move buffer
        writeb(&transmit_max, 0);
        tpos = readb(&transmit_pos);
        tmax -= tpos;
        memmove(&transmit_buf[0], &transmit_buf[tpos], tmax);
        writeb(&transmit_pos, 0);
        writeb(&transmit_max, tmax);
        tx_buffer_enable_irq();
    }
	// write header
    transmit_buf[tmax] = 0; // COBS code will be set after CRC
    transmit_buf[tmax+1] = next_sequence;
	// return buf address
    return &transmit_buf[tmax];
}

// encode (COBS) and transmit a "response" message
void tx_buffer_trigger(uint8_t *buf, uint8_t len) {
    uint_fast8_t msglen = len + MSG_MIN;

	// write trailer's crc bytes
    uint16_t crc = crc16_ccitt(buf, msglen - MSG_TRAILER_SIZE);
    buf[msglen - MSG_TRAILER_CRC + 0] = crc >> 8;
    buf[msglen - MSG_TRAILER_CRC + 1] = crc;

	// COBS encode in place
	size_t read_index  = 0;
	size_t write_index = 1;
	size_t code_index  = 0;
	uint8_t code       = 1;
	while (read_index < msglen) {
		if (buf[read_index] == MSG_SYNC) {
			buf[code_index] = code;
			code = 1;
			code_index = write_index++;
			read_index++;
		} else {
			write_index++;
			read_index++;
			code++;
			if (code == 0xFF) {
				buf[code_index] = code;
				code = 1;
				code_index = write_index++;
			}
		}
	}
	buf[code_index] = code;

	// add trailer's sync byte
	buf[msglen - MSG_TRAILER_SYNC] = MSG_SYNC;

	// start message transmit
	writeb(&transmit_max, readb(&transmit_max) + msglen);
	tx_buffer_enable_irq();

	// guard off
    writeb(&in_tx, 0);
}

// get next byte to transmit
int tx_buffer_next(uint8_t *pdata) {
	if (transmit_pos >= transmit_max)
		return -1;
	*pdata = transmit_buf[transmit_pos++];
	return 0;
}

void task_end_tx(void) {
    writeb(&in_tx, 0); // guard off
}

