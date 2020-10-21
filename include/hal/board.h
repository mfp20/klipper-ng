#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#ifdef __GIT_REVPARSE__
#define RELEASE_BOARD __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <hal/arch.h>

//
typedef void (*fptr_write_t)(uint8_t *data, uint8_t count, uint16_t timeout);

//
typedef struct commport_s commport_t;
struct commport_s {
	uint8_t type;
	uint8_t id;
	uint8_t fd;
	void *pin;
	uint32_t baud;
	uint8_t (*begin)(commport_t *port, uint32_t baud);
	uint8_t (*available)(commport_t *port);
	uint8_t (*read)(commport_t *port, uint8_t *data, uint8_t count, uint16_t timeout);
	uint8_t (*write)(commport_t *port, uint8_t *data, uint8_t count, uint16_t timeout);
	uint8_t (*end)(commport_t *port);
};

//
extern uint8_t pins[PIN_TOTAL];
extern uint8_t pins_pwm[PWM_CH];
extern uint8_t pins_adc[ADC_CH];
extern uint8_t pins_1wire[ONEWIRE_CH];
extern uint8_t pins_tty[TTY_CH][2];
extern uint8_t pins_i2c[I2C_CH][2];
extern uint8_t pins_spi[SPI_CH][4];
//
extern commport_t *port;
extern uint8_t ports_no;
extern commport_t *console;

// console&stdout&stderr
extern commport_t *binConsole; // serial port for binary control messages
extern commport_t *txtConsole; // serial port for text messages and user interactive console
extern commport_t *errConsole; // serial port for text error messages
extern commport_t *peering; // serial port for p2p messages with other MCUs (ex: sync)

// COMM PORTS

// TTY
uint8_t tty_begin(commport_t *cp, uint32_t baud);
uint8_t tty_available(commport_t *cp);
uint8_t tty_read(commport_t *cp, uint8_t *data, uint8_t count, uint16_t timeout);
uint8_t tty_write(commport_t *cp, uint8_t *data, uint8_t count, uint16_t timeout);
uint8_t tty_end(commport_t *cp);
// TODO Same for: onewire, i2c, spi

commport_t* commport_register(uint8_t type, uint8_t no);
void consolePrint(const char *format, ...);
void stdoutPrint(const char *format, ...);
void stderrPrint(const char *format, ...);
void peerPrint(const char *format, ...);
void commports_reset(void);

// init board (NOTE: not MCU, see arch_init() for MCU init)
void _board_init(void);
void board_init(void);

// tasks to run every loop cycle
void _board_run(void);
void board_run(void);

//
void _board_reset(void);
uint8_t board_reset(void);

#ifdef __cplusplus
}
#endif

#endif

