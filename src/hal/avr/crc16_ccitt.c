
#include <util/crc16.h> // _crc_ccitt_update

#include "crc16_ccitt.h" // dynmem_start

// Optimized crc16_ccitt for the avr processor
uint16_t crc16_ccitt(uint8_t *buf, uint_fast8_t len) {
    uint16_t crc = 0xFFFF;
    while (len--)
        crc = _crc_ccitt_update(crc, *buf++);
    return crc;
}

