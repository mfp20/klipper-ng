#ifndef __CMD_H
#define __CMD_H

#include <stddef.h>
#include <stdint.h> // uint8_t
#include <stdarg.h> // va_list

#include "platform.h"

// Flags for command handler declarations.
#define HF_IN_SHUTDOWN   0x01   // Handler can run even when in emergency stop

typedef void (*voidPtr)(void);
typedef uint8_t* (*cmdPtr)(uint8_t*,uint8_t*);

voidPtr initFunc[FUNCS_INIT_NO];
voidPtr taskFunc[FUNCS_TASK_NO];
voidPtr endFunc[FUNCS_END_NO];
cmdPtr cmdFunc[TYPE_NO];

void funcs_init(void);

#endif // cmd.h

