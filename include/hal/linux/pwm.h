#ifndef __LINUX_PWM_H
#define __LINUX_PWM_H

#include <stdint.h> // uint8_t

struct gpio_pwm {
    int fd;
    uint32_t period;
};
struct gpio_pwm gpio_pwm_setup(uint8_t pin, uint32_t cycle_time, uint16_t val);
void gpio_pwm_write(struct gpio_pwm g, uint16_t val);

#endif // pwm.h

