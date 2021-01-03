#ifndef __AVR_SPI_H
#define __AVR_SPI_H

#include <stdint.h>

struct spi_config {
    uint8_t spcr, spsr;
};
struct spi_config spi_hw_setup(uint32_t bus, uint8_t mode, uint32_t rate);
void spi_hw_prepare(struct spi_config config);
void spi_hw_transfer(struct spi_config config, uint8_t receive_data, uint8_t len, uint8_t *data);

#endif // spi.h

