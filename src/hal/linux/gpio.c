// Very basic shift-register support via a Linux SPI device
//
// Copyright (C) 2017-2018  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <inttypes.h>

#include "console.h" // report_errno
#include "sched.h" // shutdown

// Dummy versions of gpio_out functions
struct gpio_out gpio_out_setup(uint8_t pin, uint8_t val) {
    sched_shutdown("gpio_out_setup not supported");
}

void gpio_out_toggle_noirq(struct gpio_out g) {
}

void gpio_out_toggle(struct gpio_out g) {
}

void gpio_out_write(struct gpio_out g, uint8_t val) {
}

struct gpio_in gpio_in_setup(uint8_t pin, int8_t pull_up) {
}

void gpio_in_reset(struct gpio_in g, int8_t pull_up) {
}

uint8_t gpio_in_read(struct gpio_in g) {
}

