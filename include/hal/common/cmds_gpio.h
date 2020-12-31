#ifndef __GPIOCMDS_H
#define __GPIOCMDS_H

#include <stdint.h> // uint32_t

void digital_out_shutdown(void);
void soft_pwm_shutdown(void);

void command_config_digital_out(uint32_t*);
void command_schedule_digital_out(uint32_t*);
void command_update_digital_out(uint32_t*);
void command_set_digital_out(uint32_t*);
void command_config_soft_pwm_out(uint32_t*);
void command_schedule_soft_pwm_out(uint32_t*);

#endif // gpiocmds.h
