// AVR timer interrupt scheduling code.
//
// Copyright (C) 2016,2017  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <avr/interrupt.h> // TCNT1

#include "platform.h"

#include "sched.h" // sched_timer_dispatch


/****************************************************************
 * Low level timer code
 ****************************************************************/

// Return the number of clock ticks for a given number of microseconds
uint32_t timer_from_us(uint32_t us) {
    return us * (CONFIG_CLOCK_FREQ / 1000000);
}


static inline uint16_t timer_get(void) {
    return TCNT1;
}

static inline void timer_set(uint16_t next) {
    OCR1A = next;
}

static inline void timer_repeat_set(uint16_t next) {
    // Timer1B is used to limit the number of timers run from a timer1A irq
    OCR1B = next;
    // This is "TIFR1 = 1<<OCF1B" - gcc handles that poorly, so it's hand coded
    uint8_t dummy;
    asm volatile("ldi %0, %2\n    out %1, %0"
                 : "=d"(dummy) : "i"(&TIFR1 - 0x20), "i"(1<<OCF1B));
}

// Activate timer dispatch as soon as possible
void timer_kick(void) {
    timer_set(timer_get() + 50);
    TIFR1 = 1<<OCF1A;
}

static struct timer_s wrap_timer;

void task_end_timer(void) {
    sched_add_timer(&wrap_timer);
}

void task_init_timer(void) {
    irqstatus_t flag = irq_save();
    // no outputs
    TCCR1A = 0;
    // Normal Mode
    TCCR1B = 1<<CS10;
    // Setup for first irq
    TCNT1 = 0;
    timer_kick();
    timer_repeat_set(timer_get() + 50);
    task_end_timer();
    TIFR1 = 1<<TOV1;
    // enable interrupt
    TIMSK1 = 1<<OCIE1A;
    irq_restore(flag);
}


/****************************************************************
 * 32bit timer wrappers
 ****************************************************************/

static uint16_t timer_high;

// Return the current time (in absolute clock ticks).
uint32_t timer_read_time(void) {
    irqstatus_t flag = irq_save();
    union u32_u calc = { .val = timer_get() };
    calc.hi = timer_high;
    if (unlikely(TIFR1 & (1<<TOV1))) {
        irq_restore(flag);
        if (calc.b1 < 0xff)
            calc.hi++;
        return calc.val;
    }
    irq_restore(flag);
    return calc.val;
}

// Timer that runs every ~2ms - allows 16bit comparison optimizations
static uint_fast8_t timer_event(struct timer_s *t) {
    union u32_u *nextwake = (void*)&wrap_timer.waketime;
    if (TIFR1 & (1<<TOV1)) {
        // Hardware timer has overflowed - update overflow counter
        TIFR1 = 1<<TOV1;
        timer_high++;
        *nextwake = (union u32_u){ .hi = timer_high, .lo = 0x8000 };
    } else {
        *nextwake = (union u32_u){ .hi = timer_high + 1, .lo = 0x0000 };
    }
    return SF_RESCHEDULE;
}

static struct timer_s wrap_timer = {
    .func = timer_event,
    .waketime = 0x8000,
};

#define TIMER_IDLE_REPEAT_TICKS 8000
#define TIMER_REPEAT_TICKS 3000

#define TIMER_MIN_ENTRY_TICKS 44
#define TIMER_MIN_EXIT_TICKS 47
#define TIMER_MIN_TRY_TICKS (TIMER_MIN_ENTRY_TICKS + TIMER_MIN_EXIT_TICKS)
#define TIMER_DEFER_REPEAT_TICKS 256

// Hardware timer IRQ handler - dispatch software timers
ISR(TIMER1_COMPA_vect) {
    uint16_t next;
    for (;;) {
        // Run the next software timer
        next = sched_timer_dispatch();

        for (;;) {
            int16_t diff = timer_get() - next;
            if (likely(diff >= 0)) {
                // Another timer is pending - briefly allow irqs and then run it
                irq_enable();
                if (unlikely(TIFR1 & (1<<OCF1B)))
                    goto check_defer;
                irq_disable();
                break;
            }

            if (likely(diff <= -TIMER_MIN_TRY_TICKS))
                // Schedule next timer normally
                goto done;

            irq_enable();
            if (unlikely(TIFR1 & (1<<OCF1B)))
                goto check_defer;
            irq_disable();
            continue;

        check_defer:
            // Check if there are too many repeat timers
            irq_disable();
            uint16_t now = timer_get();
            if ((int16_t)(next - now) < (int16_t)(-timer_from_us(1000)))
                sched_try_shutdown(ERROR_TIMER_RESCHED_PAST);
            if (sched_tasks_busy()) {
                timer_repeat_set(now + TIMER_REPEAT_TICKS);
                next = now + TIMER_DEFER_REPEAT_TICKS;
                goto done;
            }
            timer_repeat_set(now + TIMER_IDLE_REPEAT_TICKS);
            timer_set(now);
        }
    }

done:
    timer_set(next);
}

