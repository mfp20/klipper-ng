#ifndef __I2CCMDS_H
#define __I2CCMDS_H

#include <stdint.h> // uint32_t

void command_config_i2c(uint32_t*);
void command_i2c_modify_bits(uint32_t*);
void command_i2c_read(uint32_t*);
void command_i2c_write(uint32_t*);

#endif // i2ccmds.h
