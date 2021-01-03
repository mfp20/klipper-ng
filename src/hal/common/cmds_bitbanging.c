// PWM, SPI, ..., software emulation
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.


#include "cmds_bitbanging.h"

#include "cmds_base.h" // alloc_chunk


/****************************************************************
 * Soft PWM output pins
 ****************************************************************/

enum {
    SPF_ON=1<<0, SPF_TOGGLING=1<<1, SPF_CHECK_END=1<<2, SPF_HAVE_NEXT=1<<3,
    SPF_NEXT_ON=1<<4, SPF_NEXT_TOGGLING=1<<5, SPF_NEXT_CHECK_END=1<<6,
};

static uint_fast8_t soft_pwm_load_event(struct timer_s *timer);

// Normal pulse change event
static uint_fast8_t soft_pwm_toggle_event(struct timer_s *timer) {
    struct pwm_software_s *s = container_of(timer, struct pwm_software_s, timer);
    gpio_out_toggle_noirq(s->pin);
    s->flags ^= SPF_ON;
    uint32_t waketime = s->timer.waketime;
    if (s->flags & SPF_ON)
        waketime += s->on_duration;
    else
        waketime += s->off_duration;
    if (s->flags & SPF_CHECK_END && !timer_is_before(waketime, s->end_time)) {
        // End of normal pulsing - next event loads new pwm settings
        s->timer.func = soft_pwm_load_event;
        waketime = s->end_time;
    }
    s->timer.waketime = waketime;
    return SF_RESCHEDULE;
}

// Load next pwm settings
static uint_fast8_t soft_pwm_load_event(struct timer_s *timer) {
    struct pwm_software_s *s = container_of(timer, struct pwm_software_s, timer);
    if (!(s->flags & SPF_HAVE_NEXT))
        sched_shutdown(ERROR_MISSED_PWM_EVENT);
    uint8_t flags = s->flags >> 4;
    s->flags = flags;
    gpio_out_write(s->pin, flags & SPF_ON);
    if (!(flags & SPF_TOGGLING)) {
        // Pin is in an always on (value=256) or always off (value=0) state
        if (!(flags & SPF_CHECK_END))
            return SF_DONE;
        s->timer.waketime = s->end_time = s->end_time + s->max_duration;
        return SF_RESCHEDULE;
    }
    // Schedule normal pin toggle timer events
    s->timer.func = soft_pwm_toggle_event;
    s->off_duration = s->next_off_duration;
    s->on_duration = s->next_on_duration;
    s->timer.waketime = s->end_time + s->on_duration;
    s->end_time += s->max_duration;
    return SF_RESCHEDULE;
}


/****************************************************************
 * Soft SPI
 ****************************************************************/

void spi_sw_prepare(struct spi_software_s *ss) {
    gpio_out_write(ss->sclk, ss->mode & 0x02);
}

void spi_sw_transfer(struct spi_software_s *ss, uint8_t receive_data, uint8_t len, uint8_t *data) {
    while (len--) {
        uint8_t outbuf = *data;
        uint8_t inbuf = 0;
        for (uint_fast8_t i = 0; i < 8; i++) {
            if (ss->mode & 0x01) {
                // MODE 1 & 3
                gpio_out_toggle(ss->sclk);
                gpio_out_write(ss->mosi, outbuf & 0x80);
                outbuf <<= 1;
                gpio_out_toggle(ss->sclk);
                inbuf <<= 1;
                inbuf |= gpio_in_read(ss->miso);
            } else {
                // MODE 0 & 2
                gpio_out_write(ss->mosi, outbuf & 0x80);
                outbuf <<= 1;
                gpio_out_toggle(ss->sclk);
                inbuf <<= 1;
                inbuf |= gpio_in_read(ss->miso);
                gpio_out_toggle(ss->sclk);
            }
        }

        if (receive_data)
            *data = inbuf;
        data++;
    }
}


/****************************************************************
 * tasks and commands
 ****************************************************************/

void task_end_soft_pwm(void) {
    uint8_t i;
    struct pwm_software_s *s;
    foreach_oid(i, s, command_config_soft_pwm_out) {
        gpio_out_write(s->pin, s->default_value);
        s->flags = s->default_value ? SPF_ON : 0;
    }
}

void command_config_soft_pwm_out(uint32_t *args) {
    struct gpio_out pin = gpio_out_setup(args[1], !!args[3]);
    struct pwm_software_s *s = oid_alloc(args[0], command_config_soft_pwm_out, sizeof(*s));
    s->pin = pin;
    s->cycle_time = args[2];
    s->default_value = !!args[4];
    s->max_duration = args[5];
    s->flags = s->default_value ? SPF_ON : 0;
}

void command_schedule_soft_pwm_out(uint32_t *args) {
    struct pwm_software_s *s = oid_lookup(args[0], command_config_soft_pwm_out);
    uint32_t time = args[1], next_on_duration = args[2], next_off_duration;
    uint8_t next_flags = SPF_CHECK_END | SPF_HAVE_NEXT;
    if (next_on_duration == 0 || next_on_duration >= s->cycle_time) {
        next_flags |= next_on_duration ? SPF_NEXT_ON : 0;
        if (!!next_on_duration != s->default_value && s->max_duration)
            next_flags |= SPF_NEXT_CHECK_END;
        next_on_duration = next_off_duration = 0;
    } else {
        next_off_duration = s->cycle_time - next_on_duration;
        next_flags |= SPF_NEXT_ON | SPF_NEXT_TOGGLING;
        if (s->max_duration)
            next_flags |= SPF_NEXT_CHECK_END;
    }
    irq_disable();
    if (s->flags & SPF_CHECK_END && timer_is_before(s->end_time, time))
        sched_shutdown(ERROR_NEXT_PWM_EXTENDS);
    s->end_time = time;
    s->next_on_duration = next_on_duration;
    s->next_off_duration = next_off_duration;
    s->flags = (s->flags & 0xf) | next_flags;
    if (s->flags & SPF_TOGGLING && timer_is_before(s->timer.waketime, time)) {
        // soft_pwm_toggle_event() will schedule a load event when ready
    } else {
        // Schedule the loading of the pwm parameters at the requested time
        sched_del_timer(&s->timer);
        s->timer.waketime = time;
        s->timer.func = soft_pwm_load_event;
        sched_add_timer(&s->timer);
    }
    irq_enable();
}

