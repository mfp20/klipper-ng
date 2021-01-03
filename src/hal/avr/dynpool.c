
#include <avr/io.h> // AVR_STACK_POINTER_REG

#include "autoconf.avr.h" // CONFIG_AVR_STACK_SIZE
#include "macro_compiler.h"
#include "dynpool.h" // dynmem_start

extern char _end;

// Return the start of memory available for dynamic allocations
void *dynmem_start(void) {
    return &_end;
}

// Return the end of memory available for dynamic allocations
void *dynmem_end(void) {
    return (void*)ALIGN(AVR_STACK_POINTER_REG, 256) - CONFIG_AVR_STACK_SIZE;
}

