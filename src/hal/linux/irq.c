// Handling of irq on linux systems
//
// Copyright (C) 2017  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "generic_irq.h"
#include "timer.h"
#include "console.h"

void irq_disable(void) {
}

void irq_enable(void) {
}

irqstatus_t irq_save(void) {
    return 0;
}

void irq_restore(irqstatus_t flag) {
}

void irq_wait(void) {
    console_sleep(next_wake_time);
}

void irq_poll(void) {
    if (!timespec_is_before(timespec_read(), next_wake_time))
        timer_dispatch();
}

