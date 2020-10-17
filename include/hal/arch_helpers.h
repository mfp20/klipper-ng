
// TYPEDEFs

// 1ms pulsing frame
// 	- 8 max pulsing pins (each pulse element is a 8 bit value, each bit represent 1 pin value)
// 	- 250 max pulses per millisecond (ie: 1 each 4us)
typedef struct pin_frame_s {
	volatile uint16_t milli;
	volatile uint8_t *pulse;
	volatile uint8_t pulse_counter;
	volatile struct pin_frame_s *next;
} pin_frame_t;
typedef struct pin_pulsing_s {
	volatile uint8_t pin[8];
	volatile pin_frame_t *head;
	volatile pin_frame_t *tail;
} pin_pulsing_t;
//
typedef struct commport_s commport_t;
struct commport_s {
	uint8_t type;
	uint8_t no;
	uint8_t fd;
	uint32_t baud;
	int (*begin)(commport_t *port, uint32_t baud);
	uint8_t (*available)(commport_t *port);
	int (*read)(commport_t *port, uint8_t *data, uint8_t count, uint16_t timeout);
	int (*write)(commport_t *port, uint8_t *data, uint8_t count, uint16_t timeout);
	int (*end)(commport_t *port);
};


// VARs

// time since boot
extern volatile uint16_t tnano;
extern volatile uint16_t tmicro;
extern volatile uint16_t tmilli;
extern volatile uint16_t tsecond;
// adc
extern volatile uint8_t adc_enable[16];
extern volatile uint8_t adc_value[16];
extern volatile uint8_t adc_current;
// pulsing pins
extern volatile pin_pulsing_t pulsing;
//
extern commport_t *ports;
extern uint8_t ports_no;
extern commport_t *console;


// FUNCTIONs

// TIME
uint16_t nanos(void);
uint16_t micros(void);
uint16_t millis(void);
uint16_t seconds(void);
uint16_t uelapsed(uint16_t mstart, uint16_t ustart, uint16_t uend, uint16_t mend);

// PINS
// Set pin to input/output mode.
void pin_mode(uint8_t pin, uint8_t mode);
// Return current pin state.
uint8_t pin_read(uint8_t pin);
// Set pin low(0). Shorthand for write(LOW).
void pin_set_low(uint8_t pin);
// Set pin high(1). Shorthand for write(HIGH).
void pin_set_high(uint8_t pin);
// Toggle pin state. Shorthand for write(!read()).
void pin_toggle(uint8_t pin);
// Set pin to given state. Non-zero value will set the pin HIGH, and zero value will set the pin LOW.
void pin_write(uint8_t pin, uint8_t value);
// Used with input() to activate internal pullup resistor on
void pin_pullup_enable(uint8_t pin);
// Open-drain pin. Use input() for high and output() for low.
void pin_open_drain(uint8_t pin);
// read an 8 bit port
unsigned char pin_port_read(uint8_t port, uint8_t bitmask);
// write an 8 bit port, only touch pins specified by a bitmask
unsigned char pin_port_write(uint8_t port, uint8_t value, uint8_t bitmask);

// PULSING
// Detect a single pulse and return its width in micro-seconds.
uint16_t pin_pulse_detect(uint8_t pin);
// Generate a single pulse with given width in micro-seconds.
void pin_pulse_single(uint8_t pin, uint16_t width);
// Add 1 frame at 'milli(seconds)' to pulsing queue
volatile pin_frame_t* pin_pulse_multi(uint16_t milli);


// COMM PORTS
// register a new commport_t
commport_t* commport_register(uint8_t type, uint8_t no);
// setup device and return the FD
uint8_t uart_begin(commport_t *uart, uint32_t baud);
// return 1 if data available to read, else 0
uint8_t uart_available(commport_t *uart);
// read count bytes from data within timeout
uint8_t uart_read(commport_t *uart, uint8_t *data, uint8_t count, uint16_t timeout);
// write count bytes from data within timeout
uint8_t uart_write(commport_t *uart, uint8_t *data, uint8_t count, uint16_t timeout);
// close the device
uint8_t uart_end(commport_t *uart);
// TODO Same for: onewire, i2c, spi


// ARCH
void vars_reset(void);
void commports_reset(void);

// console&stdout&stderr
void consoleWrite(const char *format, ...);
void logWrite(const char *format, ...);
void errWrite(const char *format, ...);

// init MCU
void _arch_init(void);
void arch_init(void);

// tasks to run every loop cycle
void _arch_run(void);
void arch_run(void);

// set MCU to know state (ie: ready for init)
void _arch_reset(void);
uint8_t arch_reset(void);

// halt MCU (ie: need to power cycle before init)
uint8_t arch_halt(void);

