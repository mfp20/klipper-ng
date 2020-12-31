#ifndef __AVR_H
#define __AVR_H

#include <inttypes.h>

void prescaler_init(void);

void *dynmem_start(void);
void *dynmem_end(void);

uint16_t crc16_ccitt(uint8_t *buf, uint_fast8_t len);

#endif // avr.h

