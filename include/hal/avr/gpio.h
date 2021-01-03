#ifndef __AVR_GPIO_H
#define __AVR_GPIO_H

#include <stdint.h>

struct gpio_out {
    struct gpio_digital_regs *regs;
    // gcc (pre v6) does better optimization when uint8_t are bitfields
    uint8_t bit : 8;
};
struct gpio_out gpio_out_setup(uint8_t pin, uint8_t val);
void gpio_out_reset(struct gpio_out g, uint8_t val);
void gpio_out_toggle_noirq(struct gpio_out g);
void gpio_out_toggle(struct gpio_out g);
void gpio_out_write(struct gpio_out g, uint8_t val);

struct gpio_in {
    struct gpio_digital_regs *regs;
    uint8_t bit;
};
struct gpio_in gpio_in_setup(uint8_t pin, int8_t pull_up);
void gpio_in_reset(struct gpio_in g, int8_t pull_up);
uint8_t gpio_in_read(struct gpio_in g);

#endif // gpio.h

