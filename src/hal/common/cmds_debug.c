// Debugging commands.
//
// Copyright (C) 2016-2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "sched.h" // sched_add_timer
#include "cmd.h" //
#include "common_io.h" // readl

#include "irq.h" // irq_save


/****************************************************************
 * Group commands
 ****************************************************************/

static struct timer group_timer;

static uint_fast8_t
group_end_event(struct timer *timer)
{
    sched_shutdown(ERROR_MISSED_EVENT);
}

void
command_start_group(uint32_t *args)
{
    sched_del_timer(&group_timer);
    group_timer.func = group_end_event;
    group_timer.waketime = args[0];
    sched_add_timer(&group_timer);
}

void
command_end_group(uint32_t *args)
{
    sched_del_timer(&group_timer);
}


/****************************************************************
 * Register debug commands
 ****************************************************************/

void
command_debug_read(uint32_t *args)
{
    uint8_t order = args[0];
    void *ptr = (void*)(size_t)args[1];
    uint32_t v;
    irqstatus_t flag = irq_save();
    switch (order) {
    default: case 0: v = readb(ptr); break;
    case 1:          v = readw(ptr); break;
    case 2:          v = readl(ptr); break;
    }
    irq_restore(flag);
    send_response(TYPE_DEBUG_READ, v);
}

void
command_debug_write(uint32_t *args)
{
    uint8_t order = args[0];
    void *ptr = (void*)(size_t)args[1];
    uint32_t v = args[2];
    irqstatus_t flag = irq_save();
    switch (order) {
    default: case 0: writeb(ptr, v); break;
    case 1:          writew(ptr, v); break;
    case 2:          writel(ptr, v); break;
    }
    irq_restore(flag);
}

void
command_debug_ping(uint32_t *args)
{
    uint8_t len = args[0];
    char *data = (void*)(size_t)args[1];
    send_response(TYPE_DEBUG_PING, len, data);
}

void
command_debug_nop(uint32_t *args)
{
}
