#ifndef __AVR_CRC16_H
#define __AVR_CRC16_H

#include <inttypes.h>

uint16_t crc16_ccitt(uint8_t *buf, uint_fast8_t len);

#endif // crc16_ccitt.h

