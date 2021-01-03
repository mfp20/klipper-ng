// Commands for controlling hardware based pulse-width-modulator pins
//
// Copyright (C) 2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "cmds_pwm.h" //

#include "sched.h" // struct timer_s
#include "cmds_base.h" // oid_alloc

struct pwm_out_s {
    struct timer_s timer;
    struct gpio_pwm pin;
    uint32_t max_duration;
    uint16_t value, default_value;
};

static uint_fast8_t pwm_end_event(struct timer_s *timer) {
    sched_shutdown(ERROR_MISSED_PWM_EVENT);
}

static uint_fast8_t pwm_event(struct timer_s *timer) {
    struct pwm_out_s *p = container_of(timer, struct pwm_out_s, timer);
    gpio_pwm_write(p->pin, p->value);
    if (p->value == p->default_value || !p->max_duration)
        return SF_DONE;
    p->timer.waketime += p->max_duration;
    p->timer.func = pwm_end_event;
    return SF_RESCHEDULE;
}


/****************************************************************
 * tasks and commands
 ****************************************************************/

void task_end_pwm(void) {
    uint8_t i;
    struct pwm_out_s *p;
    foreach_oid(i, p, command_config_pwm_out) {
        gpio_pwm_write(p->pin, p->default_value);
    }
}

void command_config_pwm_out(uint32_t *args) {
    struct gpio_pwm pin = gpio_pwm_setup(args[1], args[2], args[3]);
    struct pwm_out_s *p = oid_alloc(args[0], command_config_pwm_out, sizeof(*p));
    p->pin = pin;
    p->default_value = args[4];
    p->max_duration = args[5];
}

void command_schedule_pwm_out(uint32_t *args) {
    struct pwm_out_s *p = oid_lookup(args[0], command_config_pwm_out);
    sched_del_timer(&p->timer);
    p->timer.func = pwm_event;
    p->timer.waketime = args[1];
    p->value = args[2];
    sched_add_timer(&p->timer);
}

void command_set_pwm_out(uint32_t *args) {
    gpio_pwm_setup(args[0], args[1], args[2]);
}

