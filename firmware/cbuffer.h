#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// - head used as write index, tail used as read index
// - max 255 bytes
// - use only N-1 elements in the N element buffer:
// 	- if head is equal to tail -> the buffer is empty
// 	- if (head + 1) is equal to tail -> the buffer is full
typedef struct cbuffer_s {
	uint8_t *data;
	uint8_t head;
	uint8_t tail;
	uint8_t len;
} cbuffer_t;

void cbuf_reset(volatile cbuffer_t *buf);
uint8_t cbuf_init(volatile cbuffer_t *buf, uint16_t len);
bool cbuf_is_empty(volatile cbuffer_t *buf);
bool cbuf_is_full(volatile cbuffer_t *buf);
uint8_t cbuf_push(volatile cbuffer_t *buf, uint8_t *data, uint8_t len);
uint8_t cbuf_pop(volatile cbuffer_t *buf, uint8_t *data, uint8_t len);

