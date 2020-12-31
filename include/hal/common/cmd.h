#ifndef __CMD_H
#define __CMD_H

#include <stddef.h>
#include <stdint.h> // uint8_t
#include <stdarg.h> // va_list

#include "protocol.h" // CMD_NO
#include "macro_compiler.h"

#define FUNCS_INIT_NO 6
#define FUNCS_TASK_NO 7
#define FUNCS_END_NO 12

typedef void (*voidPtr)(void);
typedef uint8_t* (*cmdPtr)(uint8_t*,uint8_t*);

extern voidPtr initFunc[FUNCS_INIT_NO];
extern voidPtr taskFunc[FUNCS_TASK_NO];
extern voidPtr endFunc[FUNCS_END_NO];
extern cmdPtr cmdFunc[TYPE_NO];

void funcs_init(void);
void send_command(uint8_t typ, ...);
void send_response(uint8_t typ, ...);

// Flags for command handler declarations.
#define HF_IN_SHUTDOWN   0x01   // Handler can run even when in emergency stop

uint8_t __always_inline str_lookup(const char *str) {
    if (__builtin_strcmp(str, "Shutdown cleared when not shutdown") == 0)
        return 2;
    if (__builtin_strcmp(str, "Timer too close") == 0)
        return 3;
    if (__builtin_strcmp(str, "sentinel timer called") == 0)
        return 4;
    if (__builtin_strcmp(str, "Invalid command") == 0)
        return 5;
    if (__builtin_strcmp(str, "Message encode error") == 0)
        return 6;
    if (__builtin_strcmp(str, "Command parser error") == 0)
        return 7;
    if (__builtin_strcmp(str, "Command request") == 0)
        return 8;
    if (__builtin_strcmp(str, "config_reset only available when shutdown") == 0)
        return 9;
    if (__builtin_strcmp(str, "oids already allocated") == 0)
        return 10;
    if (__builtin_strcmp(str, "Can't assign oid") == 0)
        return 11;
    if (__builtin_strcmp(str, "Invalid oid type") == 0)
        return 12;
    if (__builtin_strcmp(str, "Already finalized") == 0)
        return 13;
    if (__builtin_strcmp(str, "Invalid move request size") == 0)
        return 14;
    if (__builtin_strcmp(str, "Move queue empty") == 0)
        return 15;
    if (__builtin_strcmp(str, "alloc_chunks failed") == 0)
        return 16;
    if (__builtin_strcmp(str, "alloc_chunk failed") == 0)
        return 17;
    if (__builtin_strcmp(str, "Missed scheduling of next event") == 0)
        return 18;
    if (__builtin_strcmp(str, "next soft pwm extends existing pwm") == 0)
        return 19;
    if (__builtin_strcmp(str, "Missed scheduling of next pwm event") == 0)
        return 20;
    if (__builtin_strcmp(str, "Missed scheduling of next pin event") == 0)
        return 21;
    if (__builtin_strcmp(str, "Can't reset time when stepper active") == 0)
        return 22;
    if (__builtin_strcmp(str, "Invalid count parameter") == 0)
        return 23;
    if (__builtin_strcmp(str, "Stepper too far in past") == 0)
        return 24;
    if (__builtin_strcmp(str, "No next step") == 0)
        return 25;
    if (__builtin_strcmp(str, "Set stepper past maximum stepper count") == 0)
        return 26;
    if (__builtin_strcmp(str, "ADC out of range") == 0)
        return 27;
    if (__builtin_strcmp(str, "Invalid spi config") == 0)
        return 28;
    if (__builtin_strcmp(str, "Thermocouple reader fault") == 0)
        return 29;
    if (__builtin_strcmp(str, "Thermocouple ADC out of range") == 0)
        return 30;
    if (__builtin_strcmp(str, "Invalid thermocouple chip type") == 0)
        return 31;
    if (__builtin_strcmp(str, "i2c_modify_bits: Odd number of bits!") == 0)
        return 32;
    if (__builtin_strcmp(str, "Missed scheduling of next hard pwm event") == 0)
        return 33;
    if (__builtin_strcmp(str, "Invalid buttons retransmit count") == 0)
        return 34;
    if (__builtin_strcmp(str, "Set button past maximum button count") == 0)
        return 35;
    if (__builtin_strcmp(str, "Max of 8 buttons") == 0)
        return 36;
    if (__builtin_strcmp(str, "tmcuart data too large") == 0)
        return 37;
    if (__builtin_strcmp(str, "Rescheduled timer in the past") == 0)
        return 38;
    if (__builtin_strcmp(str, "Not a valid input pin") == 0)
        return 39;
    if (__builtin_strcmp(str, "Not an output pin") == 0)
        return 40;
    if (__builtin_strcmp(str, "Not a valid ADC pin") == 0)
        return 41;
    if (__builtin_strcmp(str, "Invalid spi_setup parameters") == 0)
        return 42;
    if (__builtin_strcmp(str, "Failed to send i2c start") == 0)
        return 43;
    if (__builtin_strcmp(str, "i2c timeout") == 0)
        return 44;
    if (__builtin_strcmp(str, "Unsupported i2c bus") == 0)
        return 45;
    if (__builtin_strcmp(str, "PWM already programmed at different speed") == 0)
        return 46;
    if (__builtin_strcmp(str, "Can not use timer1 for PWM; timer1 is used for timers") == 0)
        return 47;
    if (__builtin_strcmp(str, "Not a valid PWM pin") == 0)
        return 48;
    if (__builtin_strcmp(str, "Watchdog timer!") == 0)
        return 49;
    return 0xff;
}

#endif // cmd.h

