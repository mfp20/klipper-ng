#ifndef __GENERIC_SPI_H
#define __GENERIC_SPI_H

#include <stdint.h> // uint8_t

struct spi_config {
    uint32_t cfg;
};
struct spi_config spi_setup(uint32_t bus, uint8_t mode, uint32_t rate);
void spi_prepare(struct spi_config config);
void spi_transfer(struct spi_config config, uint8_t receive_data, uint8_t len, uint8_t *data);

#endif // generic_spi.h

