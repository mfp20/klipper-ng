#ifndef __CMDS_BASE_H
#define __CMDS_BASE_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t

#include "platform.h"

#define foreach_oid(pos,data,oidtype) for (pos=-1; (data=oid_next(&pos, oidtype)); )

void *alloc_chunk(size_t size);
void move_free(void *m);
void *move_alloc(void);
void move_request_size(int size);
void *oid_lookup(uint8_t oid, void *type);
void *oid_alloc(uint8_t oid, void *type, uint16_t size);
void *oid_next(uint8_t *i, void *type);
void config_reset(uint32_t *args);
void stats_update(uint32_t start, uint32_t cur);

void task_unknown(void);
void task_noop(void);
void task_init_alloc(void);
void task_end_move(void);

uint8_t *command_unknown(uint8_t *start, uint8_t *end);
uint8_t *command_noop(uint8_t *start, uint8_t *end);
uint8_t *command_allocate_oids(uint8_t *start, uint8_t *end);
uint8_t *command_get_config(uint8_t *start, uint8_t *end);
uint8_t *command_finalize_config(uint8_t *start, uint8_t *end);
uint8_t *command_get_clock(uint8_t *start, uint8_t *end);
uint8_t *command_get_uptime(uint8_t *start, uint8_t *end);
uint8_t *command_emergency_stop(uint8_t *start, uint8_t *end);
uint8_t *command_clear_shutdown(uint8_t *start, uint8_t *end);
uint8_t *command_identify(uint8_t *start, uint8_t *end);

#endif // cmds_base.h

