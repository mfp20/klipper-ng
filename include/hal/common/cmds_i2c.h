#ifndef __CMDS_I2C_H
#define __CMDS_I2C_H

#include <stdint.h> // uint32_t

#include "platform.h"

void command_config_i2c(uint32_t*);
void command_i2c_modify_bits(uint32_t*);
void command_i2c_read(uint32_t*);
void command_i2c_write(uint32_t*);

#endif // cmds_i2c.h

