#ifndef __GENERIC_PWM_H
#define __GENERIC_PWM_H

#include <stdint.h> // uint8_t

struct gpio_pwm {
    uint8_t pin;
};
struct gpio_pwm gpio_pwm_setup(uint8_t pin, uint32_t cycle_time, uint8_t val);
void gpio_pwm_write(struct gpio_pwm g, uint8_t val);

#endif // generic_pwm.h

