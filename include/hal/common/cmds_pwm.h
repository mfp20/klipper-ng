#ifndef __PWMCMDS_H
#define __PWMCMDS_H

#include <stdint.h> // uint32_t

void pwm_shutdown(void);

void command_config_pwm_out(uint32_t*);
void command_schedule_pwm_out(uint32_t*);
void command_set_pwm_out(uint32_t*);

#endif // pwmcmds.h
