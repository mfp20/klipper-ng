#ifndef __LINUX_TIMER_H
#define __LINUX_TIMER_H

#include <inttypes.h>
#include <time.h> // struct timespec

#define NSECS 1000000000
#define NSECS_PER_TICK (NSECS / CONFIG_CLOCK_FREQ)

struct timespec next_wake_time;

void timespec_update(void);
uint8_t timespec_is_before(struct timespec ts1, struct timespec ts2);
struct timespec timespec_read(void);

int timer_check_periodic(struct timespec *ts);
uint32_t timer_from_us(uint32_t us);
uint8_t timer_is_before(uint32_t time1, uint32_t time2);
uint32_t timer_read_time(void);
void timer_kick(void);

void timer_dispatch(void);

void task_init_timer(void);

#endif // timer.h

