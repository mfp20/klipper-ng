#ifndef __CMDS_BITBANGING_H
#define __CMDS_BITBANGING_H

#include <stdint.h> // uint8_t

#include "sched.h" // struct timer_s
#include "platform.h"

struct pwm_software_s {
    struct timer_s timer;
    uint32_t on_duration, off_duration, end_time;
    uint32_t next_on_duration, next_off_duration;
    uint32_t max_duration, cycle_time;
    struct gpio_out pin;
    uint8_t default_value, flags;
};

struct spi_software_s {
    struct gpio_in miso;
    struct gpio_out mosi, sclk;
    uint8_t mode;
};

void spi_sw_prepare(struct spi_software_s *ss);
void spi_sw_transfer(struct spi_software_s *ss, uint8_t receive_data, uint8_t len, uint8_t *data);

void task_end_soft_pwm(void);

void command_config_soft_pwm_out(uint32_t*);
void command_schedule_soft_pwm_out(uint32_t*);

#endif // cmds_bitbanging.h

