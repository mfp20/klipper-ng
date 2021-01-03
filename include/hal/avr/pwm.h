#ifndef __AVR_PWM_H
#define __AVR_PWM_H

#include <stdint.h>

struct gpio_pwm {
    void *reg;
    uint8_t size8;
};
struct gpio_pwm gpio_pwm_setup(uint8_t pin, uint32_t cycle_time, uint8_t val);
void gpio_pwm_write(struct gpio_pwm g, uint8_t val);

#endif // pwm.h

