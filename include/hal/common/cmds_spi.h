#ifndef __SPICMDS_H
#define __SPICMDS_H

#include <stdint.h> // uint8_t

void spidev_shutdown(void);

void command_config_spi(uint32_t*);
void command_config_spi_without_cs(uint32_t*);
void command_spi_set_bus(uint32_t*);
void command_spi_transfer(uint32_t*);
void command_spi_send(uint32_t*);
void command_config_spi_shutdown(uint32_t*);

struct spidev_s *spidev_oid_lookup(uint8_t oid);
struct spi_software;
void spidev_set_software_bus(struct spidev_s *spi, struct spi_software *ss);
void spidev_transfer(struct spidev_s *spi, uint8_t receive_data, uint8_t data_len, uint8_t *data);

#endif // spicmds.h
