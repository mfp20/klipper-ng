// Basic infrastructure commands.
//
// Copyright (C) 2016-2020  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memset

#include "cmds_base.h" // oid_lookup

#include "sched.h" // sched_clear_shutdown


/****************************************************************
 * Low level allocation
 ****************************************************************/

static void *alloc_end;

// Allocate an area of memory
void *alloc_chunk(size_t size) {
    if (alloc_end + size > dynmem_end())
        sched_shutdown(ERROR_CHUNK_FAIL);
    void *data = alloc_end;
    alloc_end += ALIGN(size, __alignof__(void*));
    memset(data, 0, size);
    return data;
}

// Allocate an array of chunks
static void *alloc_chunks(size_t size, size_t count, uint16_t *avail) {
    size_t can_alloc = 0;
    void *p = alloc_end, *end = dynmem_end();
    while (can_alloc < count && p + size <= end)
        can_alloc++, p += size;
    if (!can_alloc)
        sched_shutdown(ERROR_CHUNK_FAIL);
    void *data = alloc_chunk(p - alloc_end);
    *avail = can_alloc;
    return data;
}


/****************************************************************
 * Move queue
 ****************************************************************/

struct move_freed {
    struct move_freed *next;
};

static struct move_freed *move_free_list;
static void *move_list;
static uint16_t move_count;
static uint8_t move_item_size;

// Is the config and move queue finalized?
static int is_finalized(void) {
    return !!move_count;
}

// Free previously allocated storage from move_alloc(). Caller must
// disable irqs.
void move_free(void *m) {
    struct move_freed *mf = m;
    mf->next = move_free_list;
    move_free_list = mf;
}

// Allocate runtime storage
void *move_alloc(void) {
    irqstatus_t flag = irq_save();
    struct move_freed *mf = move_free_list;
    if (!mf)
        sched_shutdown(ERROR_MOVE_Q_EMPTY);
    move_free_list = mf->next;
    irq_restore(flag);
    return mf;
}

// Request minimum size of runtime allocations returned by move_alloc()
void move_request_size(int size) {
    if (size > UINT8_MAX || is_finalized())
        sched_shutdown(ERROR_INVALID_MOVE_SIZE);
    if (size > move_item_size)
        move_item_size = size;
}

static void move_finalize(void) {
    if (is_finalized())
        sched_shutdown(ERROR_ALREADY_FINAL);
    move_request_size(sizeof(*move_free_list));
    move_list = alloc_chunks(move_item_size, 1024, &move_count);
    task_end_move();
}


/****************************************************************
 * Generic object ids (oid)
 ****************************************************************/

struct oid_s {
    void *type, *data;
};

static struct oid_s *oids;
static uint8_t oid_count;

void *oid_lookup(uint8_t oid, void *type) {
    if (oid >= oid_count || type != oids[oid].type)
        sched_shutdown(ERROR_OID_INVALID);
    return oids[oid].data;
}

void *oid_alloc(uint8_t oid, void *type, uint16_t size) {
    if (oid >= oid_count || oids[oid].type || is_finalized())
        sched_shutdown(ERROR_OID_CANT_ASSIGN);
    oids[oid].type = type;
    void *data = alloc_chunk(size);
    oids[oid].data = data;
    return data;
}

void *oid_next(uint8_t *i, void *type) {
    uint8_t oid = *i;
    for (;;) {
        oid++;
        if (oid >= oid_count)
            return NULL;
        if (oids[oid].type == type) {
            *i = oid;
            return oids[oid].data;
        }
    }
}


/****************************************************************
 * Config CRC
 ****************************************************************/

static uint32_t config_crc;

// Attempt a full manual reset of the config
void config_reset(uint32_t *args) {
    if (! sched_is_shutdown())
        sched_shutdown(ERROR_NOT_SHUTDOWN);
    irq_disable();
    config_crc = 0;
    oid_count = 0;
    oids = NULL;
    move_free_list = NULL;
    move_list = NULL;
    move_count = move_item_size = 0;
    task_init_alloc();
    sched_timer_reset();
    sched_clear_shutdown();
    irq_enable();
}


/****************************************************************
 * Timing and load stats
 ****************************************************************/

#define SUMSQ_BASE 256

static uint32_t stats_send_time, stats_send_time_high;

void stats_update(uint32_t start, uint32_t cur) {
    static uint32_t count, sum, sumsq;
    uint32_t diff = cur - start;
    count++;
    sum += diff;
    // Calculate sum of diff^2 - be careful of integer overflow
    uint32_t nextsumsq;
    if (diff <= 0xffff) {
        nextsumsq = sumsq + DIV_ROUND_UP(diff * diff, SUMSQ_BASE);
    } else if (diff <= 0xfffff) {
        nextsumsq = sumsq + DIV_ROUND_UP(diff, SUMSQ_BASE) * diff;
    } else {
        nextsumsq = 0xffffffff;
    }
    if (nextsumsq < sumsq)
        nextsumsq = 0xffffffff;
    sumsq = nextsumsq;

    if (timer_is_before(cur, stats_send_time + timer_from_us(5000000)))
        return;
    send_response(TYPE_BASE_STATS, count, sum, sumsq);
    if (cur < stats_send_time)
        stats_send_time_high++;
    stats_send_time = cur;
    count = sum = sumsq = 0;
}


/****************************************************************
 * tasks and commands
 ****************************************************************/

void task_unknown(void) {
}

void task_noop(void) {
}

void task_init_alloc(void) {
    alloc_end = (void*)ALIGN((size_t)dynmem_start(), __alignof__(void*));
}

// move reset
void task_end_move(void) {
    if (!move_count)
        return;
    // Add everything in move_list to the free list.
    uint32_t i;
    for (i=0; i<move_count-1; i++) {
        struct move_freed *mf = move_list + i*move_item_size;
        mf->next = move_list + (i + 1)*move_item_size;
    }
    struct move_freed *mf = move_list + (move_count - 1)*move_item_size;
    mf->next = NULL;
    move_free_list = move_list;
}

uint8_t *command_unknown(uint8_t *start, uint8_t *end) {
	// TODO error
	return end;
}

uint8_t *command_noop(uint8_t *start, uint8_t *end) {
	return ++start;
}

uint8_t *command_allocate_oids(uint8_t *start, uint8_t *end) {
    if (oids)
        sched_shutdown(ERROR_OIDS_ALREADY_ALLOC);
    uint8_t count = *(++start);
    oids = alloc_chunk(sizeof(oids[0]) * count);
    oid_count = count;
	return start;
}

uint8_t *command_get_config(uint8_t *start, uint8_t *end) {
    send_response(TYPE_BASE_GET_CONFIG, is_finalized(), config_crc, move_count, sched_is_shutdown());
	return ++start;
}

uint8_t *command_finalize_config(uint8_t *start, uint8_t *end) {
    move_finalize();
    config_crc = *(++start);
	return start;
}

uint8_t *command_get_clock(uint8_t *start, uint8_t *end) {
    send_response(TYPE_BASE_GET_CLOCK, timer_read_time());
	return ++start;
}

uint8_t *command_get_uptime(uint8_t *start, uint8_t *end) {
    uint32_t cur = timer_read_time();
    uint32_t high = stats_send_time_high + (cur < stats_send_time);
    send_response(TYPE_BASE_GET_UPTIME, high, cur);
	return ++start;
}

uint8_t *command_emergency_stop(uint8_t *start, uint8_t *end) {
    sched_shutdown(ERROR_CMD_REQ);
	return ++start;
}

uint8_t *command_clear_shutdown(uint8_t *start, uint8_t *end) {
    sched_clear_shutdown();
	return ++start;
}

uint8_t *command_identify(uint8_t *start, uint8_t *end) {
	start++;
    uint32_t offset = vlq_decode(&start);
    uint8_t count = *start++;
    uint32_t isize = TYPE_NO;
    if (offset >= isize)
        count = 0;
    else if (offset + count > isize)
        count = isize - offset;
    // TODO send_response(TYPE_BASE_IDENTIFY, offset, count, &command_identify_data[offset]);
	return start;
}

