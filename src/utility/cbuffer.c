
#include <utility/cbuffer.h>
#include <stdlib.h>

void cbuf_reset(volatile cbuffer_t *buf) {
	buf->head = 0;
	buf->tail = 0;
}

uint8_t cbuf_init(volatile cbuffer_t *buf, uint16_t len) {
	buf = (cbuffer_t*)malloc(sizeof(cbuffer_t));
	buf->data = (uint8_t *)malloc(len);
	buf->len = len;
	cbuf_reset(buf);
	return 0;
}

bool cbuf_is_empty(volatile cbuffer_t *buf) {
	if (buf->head == buf->tail) return true;
	return false;
}

bool cbuf_is_full(volatile cbuffer_t *buf) {
	if (buf->head+1 == buf->tail) return true;
	return false;
}

uint8_t cbuf_push(volatile cbuffer_t *buf, uint8_t *data, uint8_t len) {
	for(uint8_t i=0;i<len;i++) {
		int next;
		next = buf->head + 1;  // next is where head will point to after this write.
		if (next >= buf->len)
			next = 0;
		if (next == buf->tail)  // if the head + 1 == tail, circular buffer is full
			return 1;
		buf->data[buf->head] = data[i];  // Load data and then move
		buf->head = next;             // head to next data offset.
		return 0;  // return success to indicate successful push.
	}
	return 0;
}

uint8_t cbuf_pop(volatile cbuffer_t *buf, uint8_t *data, uint8_t len) {
	for(uint8_t i=0;i<len;i++) {
		int next;
		if (buf->head == buf->tail)  // if the head == tail, we don't have any data
			return 1;
		next = buf->tail + 1;  // next is where tail will point to after this read.
		if(next >= buf->len)
			next = 0;
		data[i] = buf->data[buf->tail];  // Read data and then
		buf->tail = next;              // move tail to next offset.
		return 0;  // return success to indicate successful push.
	}
	return 0;
}
