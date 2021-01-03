
#include <avr/io.h> // AVR_STACK_POINTER_REG

#include "autoconf.avr.h" // CONFIG_MCU

#include "irq.h" // irq_enable

/****************************************************************
 * Misc functions
 ****************************************************************/

// Initialize the clock prescaler (if necessary)
void task_init_prescaler(void) {
    if (CONFIG_AVR_CLKPR != -1 && (uint8_t)CONFIG_AVR_CLKPR != CLKPR) {
        irqstatus_t flag = irq_save();
        CLKPR = 0x80;
        CLKPR = CONFIG_AVR_CLKPR;
        irq_restore(flag);
    }
}

