// Software SPI emulation
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

//
#include "spi_software.h"
//
#include "sched.h" // sched_shutdown
#include "cmd.h" // shutdown
#include "cmds_base.h" // alloc_chunk
#include "cmds_spi.h" // spidev_set_software_bus
//
#include "gpio.h" // gpio_out_setup

void command_spi_set_software_bus(uint32_t *args) {
    uint8_t mode = args[4];
    if (mode > 3) sched_shutdown(ERROR_SPI_CONFIG_INVALID);

    struct spidev_s *spi = spidev_oid_lookup(args[0]);
    struct spi_software *ss = alloc_chunk(sizeof(*ss));
    ss->miso = gpio_in_setup(args[1], 1);
    ss->mosi = gpio_out_setup(args[2], 0);
    ss->sclk = gpio_out_setup(args[3], 0);
    ss->mode = mode;
    spidev_set_software_bus(spi, ss);
}

void spi_software_prepare(struct spi_software *ss) {
    gpio_out_write(ss->sclk, ss->mode & 0x02);
}

void spi_software_transfer(struct spi_software *ss, uint8_t receive_data, uint8_t len, uint8_t *data) {
    while (len--) {
        uint8_t outbuf = *data;
        uint8_t inbuf = 0;
        for (uint_fast8_t i = 0; i < 8; i++) {
            if (ss->mode & 0x01) {
                // MODE 1 & 3
                gpio_out_toggle(ss->sclk);
                gpio_out_write(ss->mosi, outbuf & 0x80);
                outbuf <<= 1;
                gpio_out_toggle(ss->sclk);
                inbuf <<= 1;
                inbuf |= gpio_in_read(ss->miso);
            } else {
                // MODE 0 & 2
                gpio_out_write(ss->mosi, outbuf & 0x80);
                outbuf <<= 1;
                gpio_out_toggle(ss->sclk);
                inbuf <<= 1;
                inbuf |= gpio_in_read(ss->miso);
                gpio_out_toggle(ss->sclk);
            }
        }

        if (receive_data)
            *data = inbuf;
        data++;
    }
}
