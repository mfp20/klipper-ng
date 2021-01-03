#ifndef __AVR_TIMER_H
#define __AVR_TIMER_H_

#include <stdint.h> // uint8_t

union u32_u {
    struct { uint8_t b0, b1, b2, b3; };
    struct { uint16_t lo, hi; };
    uint32_t val;
};

void task_init_timer(void);
void task_end_timer(void);
uint32_t timer_from_us(uint32_t us);

// Return true if time1 is before time2.  Always use this function to
// compare times as regular C comparisons can fail if the counter
// rolls over.
uint8_t __always_inline timer_is_before(uint32_t time1, uint32_t time2) {
    // This asm is equivalent to:
    //     return (int32_t)(time1 - time2) < 0;
    // But gcc doesn't do a good job with the above, so it's hand coded.
    union u32_u utime1 = { .val = time1 };
    uint8_t f = utime1.b3;
    asm("    cp  %A1, %A2\n"
        "    cpc %B1, %B2\n"
        "    cpc %C1, %C2\n"
        "    sbc %0,  %D2"
        : "+r"(f) : "r"(time1), "r"(time2));
    return (int8_t)f < 0;
}

uint32_t timer_read_time(void);
void timer_kick(void);

#endif // timer.h

