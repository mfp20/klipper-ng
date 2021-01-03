// Code for crc16_ccitt
//
// Copyright (C) 2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <inttypes.h>

// Implement the standard crc "ccitt" algorithm on the given buffer
uint16_t crc16_ccitt(uint8_t *buf, uint_fast8_t len);

