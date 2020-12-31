#ifndef __DEBUGCMDS_H
#define __DEBUGCMDS_H

#include <stdint.h> // uint32_t

void command_start_group(uint32_t*);
void command_end_group(uint32_t*);
void command_debug_nop(uint32_t*);
void command_debug_ping(uint32_t*);
void command_debug_read(uint32_t*);
void command_debug_write(uint32_t*);

#endif // debugcmds.h
