// Very basic shift-register support via a Linux SPI device
//
// Copyright (C) 2017-2018  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <fcntl.h> // open
#include <linux/spi/spidev.h> // SPI_IOC_MESSAGE
#include <stdio.h> // snprintf
#include <string.h> // memset
#include <sys/ioctl.h> // ioctl
#include <unistd.h> // write

//#include "command.h" // DECL_COMMAND
#include "spi.h" // spi_setup
#include "console.h" // report_errno
#include "sched.h" // shutdown

#define SPIBUS(chip, pin) (((chip)<<8) + (pin))
#define SPIBUS_TO_BUS(spi_bus) ((spi_bus) >> 8)
#define SPIBUS_TO_DEV(spi_bus) ((spi_bus) & 0xff)

struct spi_s {
    uint32_t bus, dev;
    int fd;
};
static struct spi_s devices[16];
static int devices_count;

static int spi_open(uint32_t bus, uint32_t dev) {
    // Find existing device (if already opened)
    int i;
    for (i=0; i<devices_count; i++)
        if (devices[i].bus == bus && devices[i].dev == dev)
            return devices[i].fd;

    // Setup new SPI device
    if (devices_count >= ARRAY_SIZE(devices))
        sched_shutdown(ERROR_SPI_DEV_TOO_MANY);
    char fname[256];
    snprintf(fname, sizeof(fname), "/dev/spidev%d.%d", bus, dev);
    int fd = open(fname, O_RDWR|O_CLOEXEC);
    if (fd < 0) {
        report_errno("open spi", fd);
        sched_shutdown(ERROR_SPI_DEV_UNAVAIL);
    }
    int ret = set_non_blocking(fd);
    if (ret < 0)
        sched_shutdown(ERROR_SPI_NBLOCK_UNAVAIL);

    devices[devices_count].bus = bus;
    devices[devices_count].dev = dev;
    devices[devices_count].fd = fd;
    return fd;
}

struct spi_config spi_hw_setup(uint32_t bus, uint8_t mode, uint32_t rate) {
    int bus_id = SPIBUS_TO_BUS(bus), dev_id = SPIBUS_TO_DEV(bus);
    int fd = spi_open(bus_id, dev_id);
    int ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &rate);
    if (ret < 0) {
        report_errno("ioctl set max spi speed", ret);
        sched_shutdown(ERROR_SPI_SPEED_UNAVAIL);
    }
    return (struct spi_config) { fd , rate};
}

void spi_hw_prepare(struct spi_config config) {
}

void spi_hw_transfer(struct spi_config config, uint8_t receive_data, uint8_t len, uint8_t *data) {
    if (!len)
        return;

    if (receive_data) {
        struct spi_ioc_transfer transfer;
        memset(&transfer, 0, sizeof(transfer));
        transfer.tx_buf = (uintptr_t)data;
        transfer.rx_buf = (uintptr_t)data;
        transfer.len = len;
        transfer.speed_hz = config.rate;
        transfer.bits_per_word = 8;
        transfer.cs_change = 0;
        int ret = ioctl(config.fd, SPI_IOC_MESSAGE(1), &transfer);
        if (ret < 0) {
            report_errno("spi ioctl", ret);
            sched_try_shutdown(ERROR_SPI_IOCTL_UNAVAIL);
        }
    } else {
        int ret = write(config.fd, data, len);
        if (ret < 0) {
            report_errno("write spi", ret);
            sched_try_shutdown(ERROR_SPI_WRITE_UNAVAIL);
        }
    }
}

