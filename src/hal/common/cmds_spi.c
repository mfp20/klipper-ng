// Commands for sending messages on an SPI bus
//
// Copyright (C) 2016-2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memcpy

#include "cmds_spi.h" //

#include "cmds_base.h" // oid_alloc


/****************************************************************
 * SPI
 ****************************************************************/

enum {
    SF_HAVE_PIN = 1, SF_SOFTWARE = 2, SF_HARDWARE = 4,
};

struct spidev_shutdown_s {
    struct spidev_s *spi;
    uint8_t shutdown_msg_len;
    uint8_t shutdown_msg[];
};

struct spidev_s * spidev_oid_lookup(uint8_t oid) {
    return oid_lookup(oid, command_config_spi);
}

void spi_set_software_bus(struct spidev_s *spi, struct spi_software_s *ss) {
    if (spi->flags & (SF_SOFTWARE|SF_HARDWARE))
        sched_shutdown(ERROR_SPI_CONFIG_INVALID);
    spi->spi_software = ss;
    spi->flags |= SF_SOFTWARE;
}

void spi_transfer(struct spidev_s *spi, uint8_t receive_data, uint8_t data_len, uint8_t *data) {
    if (!(spi->flags & (SF_SOFTWARE|SF_HARDWARE)))
        // Not yet initialized
        return;

    if (CONFIG_HAVE_GPIO_BITBANGING && spi->flags & SF_SOFTWARE)
        spi_sw_prepare(spi->spi_software);
    else
        spi_hw_prepare(spi->spi_config);

    if (spi->flags & SF_HAVE_PIN)
        gpio_out_write(spi->pin, 0);

    if (CONFIG_HAVE_GPIO_BITBANGING && spi->flags & SF_SOFTWARE)
        spi_sw_transfer(spi->spi_software, receive_data, data_len, data);
    else
        spi_hw_transfer(spi->spi_config, receive_data, data_len, data);

    if (spi->flags & SF_HAVE_PIN)
        gpio_out_write(spi->pin, 1);
}


/****************************************************************
 * tasks and commands
 ****************************************************************/

void task_end_spidev(void) {
    // Cancel any transmissions that may be in progress
    uint8_t oid;
    struct spidev_s *spi;
    foreach_oid(oid, spi, command_config_spi) {
        if (spi->flags & SF_HAVE_PIN)
            gpio_out_write(spi->pin, 1);
    }

    // Send shutdown messages
    struct spidev_shutdown_s *sd;
    foreach_oid(oid, sd, command_config_spi_shutdown) {
        spi_transfer(sd->spi, 0, sd->shutdown_msg_len, sd->shutdown_msg);
    }
}

void command_config_spi(uint32_t *args) {
    struct spidev_s *spi = oid_alloc(args[0], command_config_spi, sizeof(*spi));
    spi->pin = gpio_out_setup(args[1], 1);
    spi->flags |= SF_HAVE_PIN;
}

void command_config_spi_without_cs(uint32_t *args) {
    struct spidev_s *spi = oid_alloc(args[0], command_config_spi, sizeof(*spi));
}

void command_spi_set_bus(uint32_t *args) {
    struct spidev_s *spi = spidev_oid_lookup(args[0]);
    uint8_t mode = args[2];
    if (mode > 3 || spi->flags & (SF_SOFTWARE|SF_HARDWARE))
        sched_shutdown(ERROR_SPI_CONFIG_INVALID);
    spi->spi_config = spi_hw_setup(args[1], mode, args[3]);
    spi->flags |= SF_HARDWARE;
}

void command_spi_set_software_bus(uint32_t *args) {
    uint8_t mode = args[4];
    if (mode > 3) sched_shutdown(ERROR_SPI_CONFIG_INVALID);

    struct spidev_s *spi = spidev_oid_lookup(args[0]);
    struct spi_software_s *ss = alloc_chunk(sizeof(*ss));
    ss->miso = gpio_in_setup(args[1], 1);
    ss->mosi = gpio_out_setup(args[2], 0);
    ss->sclk = gpio_out_setup(args[3], 0);
    ss->mode = mode;
    spi_set_software_bus(spi, ss);
}

void command_spi_transfer(uint32_t *args) {
    uint8_t oid = args[0];
    struct spidev_s *spi = spidev_oid_lookup(oid);
    uint8_t data_len = args[1];
    uint8_t *data = (void*)(size_t)args[2];
    spi_transfer(spi, 1, data_len, data);
    send_response(TYPE_SPI_TRANSFER, oid, data_len, data);
}

void command_spi_send(uint32_t *args) {
    struct spidev_s *spi = spidev_oid_lookup(args[0]);
    uint8_t data_len = args[1];
    uint8_t *data = (void*)(size_t)args[2];
    spi_transfer(spi, 0, data_len, data);
}

void command_config_spi_shutdown(uint32_t *args) {
    struct spidev_s *spi = spidev_oid_lookup(args[1]);
    uint8_t shutdown_msg_len = args[2];
    struct spidev_shutdown_s *sd = oid_alloc(
        args[0], command_config_spi_shutdown, sizeof(*sd) + shutdown_msg_len);
    sd->spi = spi;
    sd->shutdown_msg_len = shutdown_msg_len;
    uint8_t *shutdown_msg = (void*)(size_t)args[3];
    memcpy(sd->shutdown_msg, shutdown_msg, shutdown_msg_len);
}

