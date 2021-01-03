#ifndef __CMDS_GPIO_H
#define __CMDS_GPIO_H

#include <stdint.h> // uint32_t

#include "platform.h"

void task_end_digital_out(void);

void command_config_digital_out(uint32_t*);
void command_schedule_digital_out(uint32_t*);
void command_update_digital_out(uint32_t*);
void command_set_digital_out(uint32_t*);

#endif // cmds_gpio.h

