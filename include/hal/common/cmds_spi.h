#ifndef __CMDS_SPI_H
#define __CMDS_SPI_H

#include <stdint.h> // uint8_t

#include "cmds_bitbanging.h" // struct spi_software_s
#include "platform.h"

struct spidev_s {
    union {
        struct spi_config spi_config;
        struct spi_software_s *spi_software;
    };
    struct gpio_out pin;
    uint8_t flags;
};

struct spidev_s *spidev_oid_lookup(uint8_t oid);
void spi_set_software_bus(struct spidev_s *spi, struct spi_software_s *ss);
void spi_transfer(struct spidev_s *spi, uint8_t receive_data, uint8_t data_len, uint8_t *data);

void task_end_spidev(void);

void command_config_spi(uint32_t*);
void command_config_spi_without_cs(uint32_t*);
void command_spi_set_bus(uint32_t*);
void command_spi_set_software_bus(uint32_t*);
void command_spi_transfer(uint32_t*);
void command_spi_send(uint32_t*);
void command_config_spi_shutdown(uint32_t*);

#endif // cmds_spi.h

