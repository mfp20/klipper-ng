#ifndef __ADCCMDS_H
#define __ADCCMDS_H

#include <stdint.h> // uint32_t

void analog_in_task(void);
void analog_in_shutdown(void);

void command_config_analog_in(uint32_t*);
void command_query_analog_in(uint32_t*);

#endif // adccmds.h
