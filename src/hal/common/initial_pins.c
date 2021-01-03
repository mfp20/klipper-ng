// Support setting gpio pins at mcu start
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "initial_pins.h" // initial_pins

#include "sched.h" //

const struct initial_pin_s initial_pins[] PROGMEM = {
};

const int initial_pins_size PROGMEM = ARRAY_SIZE(initial_pins);

void task_init_initial_pins(void) {
    int i;
    for (i=0; i<initial_pins_size; i++) {
        const struct initial_pin_s *ip = &initial_pins[i];
        gpio_out_setup(READP(ip->pin), READP(ip->flags) & IP_OUT_HIGH);
    }
}

