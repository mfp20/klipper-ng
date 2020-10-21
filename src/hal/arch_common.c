
#include <hal/arch.h>
#include <stddef.h>
#include <stdlib.h>

volatile uint16_t tmicro = 0;
volatile uint16_t tmilli = 0;
volatile uint16_t tsecond = 0;

volatile uint8_t pin[PIN_TOTAL];
volatile uint8_t pin_adc_enable[16];
volatile uint8_t pin_adc_value[16];
volatile uint8_t pin_adc_current = 0;
volatile pin_pulsing_t pin_pulsing;

uint16_t uelapsed(uint16_t mstart, uint16_t ustart, uint16_t uend, uint16_t mend) {
	if (mstart==mend) { // less than 1ms, within the same millisecond interval
		return uend-ustart;
	} else if (mend==mstart+1) { // less than 1ms, across 2ms interval
		if (uend>ustart) {
			return (1000-ustart)+uend;
		} else {
			return (1000-ustart)+uend;
		}
	} else { // more than 1ms
		if (mend>mstart) {
			return ((mend-mstart-1)*1000)+((1000-ustart)+uend);
		} else {
			return (((1000-mstart)+mend+1)*1000)+((1000-ustart)+uend);
		}
	}
}

void vars_reset(void) {
	tmicro = 0;
	tmilli = 0;
	tsecond = 0;
	//
	for (uint8_t i = 0;i<8;i++) {
		pin_adc_enable[i] = 0;
		pin_adc_value[i] = 0;
	}
	//
	for (uint8_t i = 0;i<8;i++) {
		pin_pulsing.pin[i] = 0;
	}
	pin_pulsing.head = NULL;
	pin_pulsing.tail = NULL;
}

void arch_init(void) {
	vars_reset();
	_arch_init();
}

void arch_run(void) {
	_arch_run();
}

uint8_t arch_reset(void) {
	_arch_reset();
	vars_reset();
	return 0;
}

uint8_t arch_halt(void) {
	arch_reset();
	return 0;
}

