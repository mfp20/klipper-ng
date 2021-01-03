// Debugging commands.
//
// Copyright (C) 2016-2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stddef.h> // size_t

#include "cmds_debug.h" //

#include "sched.h" // sched_shutdown
#include "generic_io.h" // readl


/****************************************************************
 * Group commands
 ****************************************************************/

static struct timer_s group_timer;

static uint_fast8_t group_end_event(struct timer_s *timer) {
    sched_shutdown(ERROR_MISSED_EVENT);
}


/****************************************************************
 * tasks and commands
 ****************************************************************/

uint8_t *command_start_group(uint8_t *start, uint8_t *end) {
	start++;
    sched_del_timer(&group_timer);
    group_timer.func = group_end_event;
    group_timer.waketime = vlq_decode(&start);
    sched_add_timer(&group_timer);
	return start;
}

uint8_t *command_end_group(uint8_t *start, uint8_t *end) {
    sched_del_timer(&group_timer);
	return ++start;
}

uint8_t *command_debug_read(uint8_t *start, uint8_t *end) {
	start++;
    uint8_t order = *start++;
    void *ptr = (void*)(size_t)vlq_decode(&start);
    uint32_t v;
    irqstatus_t flag = irq_save();
    switch (order) {
    default: case 0: v = readb(ptr); break;
    case 1:          v = readw(ptr); break;
    case 2:          v = readl(ptr); break;
    }
    irq_restore(flag);
    send_response(TYPE_DEBUG_READ, v);
	return start;
}

uint8_t *command_debug_write(uint8_t *start, uint8_t *end) {
	start++;
    uint8_t order = *start++;
    void *ptr = (void*)(size_t)vlq_decode(&start);
    uint32_t v = vlq_decode(&start);
    irqstatus_t flag = irq_save();
    switch (order) {
    default: case 0: writeb(ptr, v); break;
    case 1:          writew(ptr, v); break;
    case 2:          writel(ptr, v); break;
    }
    irq_restore(flag);
	return start;
}

uint8_t *command_debug_ping(uint8_t *start, uint8_t *end) {
	start++;
    uint8_t len = *start++;
    void *data = (void*)(size_t)vlq_decode(&start);
    send_response(TYPE_DEBUG_PING, len, data);
	return start;
}

uint8_t *command_debug_nop(uint8_t *start, uint8_t *end) {
	return ++start;
}

