#ifndef __SPI_SOFTWARE_H
#define __SPI_SOFTWARE_H

#include <stdint.h> // uint8_t

#include "gpio.h" // struct gpio_in, struct gpio_out

struct spi_software {
    struct gpio_in miso;
    struct gpio_out mosi, sclk;
    uint8_t mode;
};

void command_spi_set_software_bus(uint32_t*);

struct spi_software *spi_software_oid_lookup(uint8_t oid);
void spi_software_prepare(struct spi_software *ss);
void spi_software_transfer(struct spi_software *ss, uint8_t receive_data, uint8_t len, uint8_t *data);

#endif // spi_software.h
