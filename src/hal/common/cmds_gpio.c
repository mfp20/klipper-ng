// Commands for controlling GPIO pins
//
// Copyright (C) 2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "cmds_gpio.h" //

#include "sched.h" // struct timer_s
#include "cmds_base.h" // oid_alloc


/****************************************************************
 * Digital out pins
 ****************************************************************/

struct digital_out_s {
    struct timer_s timer;
    struct gpio_out pin;
    uint32_t max_duration;
    uint8_t value, default_value;
};

static uint_fast8_t digital_end_event(struct timer_s *timer) {
    sched_shutdown(ERROR_MISSED_PIN_EVENT);
}

static uint_fast8_t digital_out_event(struct timer_s *timer) {
    struct digital_out_s *d = container_of(timer, struct digital_out_s, timer);
    gpio_out_write(d->pin, d->value);
    if (d->value == d->default_value || !d->max_duration)
        return SF_DONE;
    d->timer.waketime += d->max_duration;
    d->timer.func = digital_end_event;
    return SF_RESCHEDULE;
}


/****************************************************************
 * tasks and commands
 ****************************************************************/

void task_end_digital_out(void) {
    uint8_t i;
    struct digital_out_s *d;
    foreach_oid(i, d, command_config_digital_out) {
        gpio_out_write(d->pin, d->default_value);
    }
}

void command_config_digital_out(uint32_t *args) {
    struct gpio_out pin = gpio_out_setup(args[1], args[2]);
    struct digital_out_s *d = oid_alloc(args[0], command_config_digital_out, sizeof(*d));
    d->pin = pin;
    d->default_value = args[3];
    d->max_duration = args[4];
}

void command_schedule_digital_out(uint32_t *args) {
    struct digital_out_s *d = oid_lookup(args[0], command_config_digital_out);
    sched_del_timer(&d->timer);
    d->timer.func = digital_out_event;
    d->timer.waketime = args[1];
    d->value = args[2];
    sched_add_timer(&d->timer);
}

void command_update_digital_out(uint32_t *args) {
    struct digital_out_s *d = oid_lookup(args[0], command_config_digital_out);
    sched_del_timer(&d->timer);
    uint8_t value = args[1];
    gpio_out_write(d->pin, value);
    if (value != d->default_value && d->max_duration) {
        d->timer.waketime = timer_read_time() + d->max_duration;
        d->timer.func = digital_end_event;
        sched_add_timer(&d->timer);
    }
}

void command_set_digital_out(uint32_t *args) {
    gpio_out_setup(args[0], args[1]);
}

