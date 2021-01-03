#ifndef __CMDS_PWM_H
#define __CMDS_PWM_H

#include <stdint.h> // uint32_t

#include "platform.h"

void task_end_pwm(void);

void command_config_pwm_out(uint32_t*);
void command_schedule_pwm_out(uint32_t*);
void command_set_pwm_out(uint32_t*);

#endif // cmds_pwm.h

