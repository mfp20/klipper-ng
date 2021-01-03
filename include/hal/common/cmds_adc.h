#ifndef __CMDS_ADC_H
#define __CMDS_ADC_H

#include <stdint.h> // uint32_t

#include "platform.h"

void task_analog_in(void);
void task_end_analog_in(void);

void command_config_analog_in(uint32_t*);
void command_query_analog_in(uint32_t*);

#endif // cmds_adc.h

