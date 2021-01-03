// Commands for sending messages on an I2C bus
//
// Copyright (C) 2018  Florian Heilmann <Florian.Heilmann@gmx.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "cmds_i2c.h" //

#include "sched.h" // send_response, sched_shutdown
#include "cmds_base.h" // oid_alloc

struct i2cdev_s {
    struct i2c_config i2c_config;
};


/****************************************************************
 * tasks and commands
 ****************************************************************/

void command_config_i2c(uint32_t *args) {
    uint8_t addr = args[3] & 0x7f;
    struct i2cdev_s *i2c = oid_alloc(args[0], command_config_i2c, sizeof(*i2c));
    i2c->i2c_config = i2c_setup(args[1], args[2], addr);
}

void command_i2c_write(uint32_t *args) {
    uint8_t oid = args[0];
    struct i2cdev_s *i2c = oid_lookup(oid, command_config_i2c);
    uint8_t data_len = args[1];
    uint8_t *data = (void*)(size_t)args[2];
    i2c_write(i2c->i2c_config, data_len, data);
}

void command_i2c_read(uint32_t * args) {
    uint8_t oid = args[0];
    struct i2cdev_s *i2c = oid_lookup(oid, command_config_i2c);
    uint8_t reg_len = args[1];
    uint8_t *reg = (void*)(size_t)args[2];
    uint8_t data_len = args[3];
    uint8_t receive_array[data_len];
    uint8_t *data = (void*)(size_t)receive_array;
    i2c_read(i2c->i2c_config, reg_len, reg, data_len, data);
    send_response(TYPE_I2C_READ, oid, data_len, data);
}

void command_i2c_modify_bits(uint32_t *args) {
    uint8_t oid = args[0];
    struct i2cdev_s *i2c = oid_lookup(oid, command_config_i2c);
    uint8_t reg_len = args[1];
    uint8_t *reg = (void*)(size_t)args[2];
    uint32_t clear_set_len = args[3];
    if (clear_set_len % 2 != 0) {
        sched_shutdown(ERROR_I2C_MODIFY_BITS);
    }
    uint8_t data_len = clear_set_len/2;
    uint8_t *clear_set = (void*)(size_t)args[4];
    uint8_t receive_array[reg_len + data_len];
    uint8_t *receive_data = (void*)(size_t)receive_array;
    for (int i = 0; i < reg_len; i++) {
        *(receive_data + i) = *(reg + i);
    }
    i2c_read(i2c->i2c_config, reg_len, reg, data_len, receive_data + reg_len);
    for (int i = 0; i < data_len; i++) {
        *(receive_data + reg_len + i) &= ~(*(clear_set + i));
        *(receive_data + reg_len + i) |= *(clear_set + clear_set_len/2 + i);
    }
    i2c_write(i2c->i2c_config, reg_len + data_len, receive_array);
}

