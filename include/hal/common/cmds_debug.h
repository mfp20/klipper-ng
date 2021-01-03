#ifndef __CMDS_DEBUG_H
#define __CMDS_DEBUG_H

#include <stdint.h> // uint32_t

#include "platform.h"

uint8_t *command_start_group(uint8_t *start, uint8_t *end);
uint8_t *command_end_group(uint8_t *start, uint8_t *end);
uint8_t *command_debug_nop(uint8_t *start, uint8_t *end);
uint8_t *command_debug_ping(uint8_t *start, uint8_t *end);
uint8_t *command_debug_read(uint8_t *start, uint8_t *end);
uint8_t *command_debug_write(uint8_t *start, uint8_t *end);

#endif // cmds_debug.h

