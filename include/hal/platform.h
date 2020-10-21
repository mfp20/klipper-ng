#ifndef HAL_DEFINES_H
#define HAL_DEFINES_H

#ifdef __GIT_REVPARSE__
#define RELEASE_DEFINES __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#include <utility/macros.h>

// reset modes
#define SYSTEM_RESET_ZERO	0 // reset all pins without power cycle
#define SYSTEM_RESET_REBOOT	1 // power cycle
#define SYSTEM_RESET_HALT	2 // halt (no reboot without human intervention)
#define SYSTEM_RESET_TOTAL	3

// pin modes
#define PIN_MODE_UNUSED		0 //
#define PIN_MODE_PULLUP     1 // internal pull-up resistor enabled
#define PIN_MODE_INPUT		2 // digital in
#define PIN_MODE_OUTPUT		3 // digital out
#define PIN_MODE_PWM		4 // digital PWM out
#define PIN_MODE_ANALOG		5 // analog input
#define PIN_MODE_1WIRE		6 // pin used for 1-wire
#define PIN_MODE_TTY		7 // pin used for UART
#define PIN_MODE_I2C        8 // pin used for I2C
#define PIN_MODE_SPI		9 // pin used for SPI
#define PIN_MODE_DEBOUNCE	10 // pin used for buttons, rotary encoders, ...
#define PIN_MODE_SHIFT      11 // shiftIn/shiftOut
#define PIN_MODE_STEPPER    12 // pin used for stepper motor
#define PIN_MODE_SERVO      13 // pin used for servo motor
#define PIN_MODE_IGNORE     14 // pin ignored/disabled
#define PIN_MODE_TOTAL		15

// comm port types
#define COMMPORT_TYPE_NULL	0
#define COMMPORT_TYPE_FD	1
#define COMMPORT_TYPE_PIN	2
#define COMMPORT_TYPE_1WIRE 3
#define COMMPORT_TYPE_TTY	4
#define COMMPORT_TYPE_I2C	5
#define COMMPORT_TYPE_SPI	6
#define COMMPORT_TYPE_TOTAL	7

// comm port errors
#define COMMPORT_ERROR_NO		0 // no error
#define COMMPORT_ERROR_FRAME	1 // Frame Error (FEn)
#define COMMPORT_ERROR_DATAOVR	2 // Data OverRun (DORn)
#define COMMPORT_ERROR_PARITY	3 // Parity Error (UPEn)
#define COMMPORT_ERROR_BUFOVF	4 // Buffer overflow
#define COMMPORT_ERROR_TOTAL	5

//
#if defined(__FIRMWARE_ARCH_BOGUS__)
#define POINTER_SIZE		8		// memory address size
#define F_CPU				16000000UL  //
#define RAM					8		// ram kilobytes
#define ROM					4		// rom kilobytes
#define TIMER8				1		// 8bit timers
#define TIMER16				1		// 16bit timers
#define TIMER32				1		// 32bit timers
#define WATCHDOG			1		//
#define PIN_TOTAL			8		// total amount of I/O lines
enum pin_e {
    P1 = 0,
    P2 = 1,
    P3 = 2,
    P4 = 3,
    P5 = 4,
    P6 = 5,
    P7 = 6,
    P8 = 7,
};

#elif defined(__FIRMWARE_ARCH_LINUX__)
#define POINTER_SIZE		64		// memory address size
#define F_CPU				1000000000ULL	// 1GHz
#define RAM					8000000UL		// 8GB RAM
#define ROM					8000000UL		// 8GB disk
#define TIMER8				0		// 8bit timers
#define TIMER16				0		// 16bit timers
#define TIMER32				3		// 32bit timers (3 64 bit timers per process)
#define WATCHDOG			1		//
#define PIN_TOTAL			0		// total amount of I/O lines
enum pin_e {
	None = 0,
};

#elif defined(__FIRMWARE_ARCH_ATMEGA328__) || defined(__FIRMWARE_ARCH_ATMEGA328P__)
#define POINTER_SIZE		8		// memory address size
#define F_CPU				16000000UL  // cpu@16Mhz: we have 1 cycle each 62.5ns. 16 cycles per 1us.
#define RAM					2		// ram kilobytes
#define ROM					1		// rom kilobytes
#define TIMER8				2		// 8bit timers
#define TIMER16				1		// 16bit timers
#define TIMER32				0		// 32bit timers
#define WATCHDOG			1		//
#define PIN_TOTAL			23		// total amount of I/O lines
enum pin_e {
    P1 = GPIO_PIN(0x23,0),	//!< PINB:0
    P2 = GPIO_PIN(0x23,1),	//!< PINB:1
    P3 = GPIO_PIN(0x23,2),	//!< PINB:2/SS
    P4 = GPIO_PIN(0x23,3),	//!< PINB:3/MOSI/ICSP.4
    P5 = GPIO_PIN(0x23,4),	//!< PINB:4/MISO/ICSP.1
    P6 = GPIO_PIN(0x23,5),	//!< PINB:5/SCK/ICSP.3
    P7 = 0,
    P8 = 0,

    P9 = GPIO_PIN(0x26,0),	//!< PINC:0/A0
    P10 = GPIO_PIN(0x26,1),	//!< PINC:1/A1
    P11 = GPIO_PIN(0x26,2),	//!< PINC:2/A2
    P12 = GPIO_PIN(0x26,3),	//!< PINC:3/A3
    P13 = GPIO_PIN(0x26,4),	//!< PINC:4/A4/SDA
    P14 = GPIO_PIN(0x26,5),	//!< PINC:5/A5/SCL
    P15 = 0,
    P16 = 0,

    P17 = GPIO_PIN(0x29,0),	//!< PIND:0
    P18 = GPIO_PIN(0x29,1),	//!< PIND:1
    P19 = GPIO_PIN(0x29,2),	//!< PIND:2
    P20 = GPIO_PIN(0x29,3),	//!< PIND:3
    P21 = GPIO_PIN(0x29,4),	//!< PIND:4
    P22 = GPIO_PIN(0x29,5),	//!< PIND:5
    P23 = GPIO_PIN(0x29,6),	//!< PIND:6
    P24 = GPIO_PIN(0x29,7),	//!< PIND:7
};

#elif defined(__FIRMWARE_ARCH_ATMEGA644__) || defined(__FIRMWARE_ARCH_ATMEGA644P__)
#define POINTER_SIZE		8		// memory address size
#define F_CPU				16000000UL  // cpu@16Mhz: we have 1 cycle each 62.5ns. 16 cycles per 1us.
#define RAM					4		// ram kilobytes
#define ROM					2		// rom kilobytes
#define TIMER8				2		// 8bit timers
#define TIMER16				1		// 16bit timers
#define TIMER32				0		// 32bit timers
#define WATCHDOG			1		//
#define PIN_TOTAL			32		// total amount of I/O lines
enum pin_e {
    P1 = GPIO_PIN(0x23,0),	//!< PINB:0
    P2 = GPIO_PIN(0x23,1),	//!< PINB:1
    P3 = GPIO_PIN(0x23,2),	//!< PINB:2/SS
    P4 = GPIO_PIN(0x23,3),	//!< PINB:3/MOSI/ICSP.4
    P5 = GPIO_PIN(0x23,4),	//!< PINB:4/MISO/ICSP.1
    P6 = GPIO_PIN(0x23,5),	//!< PINB:5/SCK/ICSP.3
    P7 = 0,
    P8 = 0,

    P9 = GPIO_PIN(0x26,0),	//!< PINC:0/A0
    P10 = GPIO_PIN(0x26,1),	//!< PINC:1/A1
    P11 = GPIO_PIN(0x26,2),	//!< PINC:2/A2
    P12 = GPIO_PIN(0x26,3),	//!< PINC:3/A3
    P13 = GPIO_PIN(0x26,4),	//!< PINC:4/A4/SDA
    P14 = GPIO_PIN(0x26,5),	//!< PINC:5/A5/SCL
    P15 = 0,
    P16 = 0,

    P17 = GPIO_PIN(0x29,0),	//!< PIND:0
    P18 = GPIO_PIN(0x29,1),	//!< PIND:1
    P19 = GPIO_PIN(0x29,2),	//!< PIND:2
    P20 = GPIO_PIN(0x29,3),	//!< PIND:3
    P21 = GPIO_PIN(0x29,4),	//!< PIND:4
    P22 = GPIO_PIN(0x29,5),	//!< PIND:5
    P23 = GPIO_PIN(0x29,6),	//!< PIND:6
    P24 = GPIO_PIN(0x29,7),	//!< PIND:7
};

#elif defined(__FIRMWARE_ARCH_ATMEGA1284P__)
#define POINTER_SIZE		8		// memory address size
#define F_CPU				16000000UL  // cpu@16Mhz: we have 1 cycle each 62.5ns. 16 cycles per 1us.
#define RAM					16		// ram kilobytes
#define ROM					4		// rom kilobytes
#define TIMER8				2		// 8bit timers
#define TIMER16				2		// 16bit timers
#define TIMER32				0		// 32bit timers
#define WATCHDOG			1		//
#define PIN_TOTAL			32		// total amount of I/O lines
enum pin_e {
    P1 = GPIO_PIN(0x23,0),	//!< PINB:0
    P2 = GPIO_PIN(0x23,1),	//!< PINB:1
    P3 = GPIO_PIN(0x23,2),	//!< PINB:2/SS
    P4 = GPIO_PIN(0x23,3),	//!< PINB:3/MOSI/ICSP.4
    P5 = GPIO_PIN(0x23,4),	//!< PINB:4/MISO/ICSP.1
    P6 = GPIO_PIN(0x23,5),	//!< PINB:5/SCK/ICSP.3
    P7 = 0,
    P8 = 0,

    P9 = GPIO_PIN(0x26,0),	//!< PINC:0/A0
    P10 = GPIO_PIN(0x26,1),	//!< PINC:1/A1
    P11 = GPIO_PIN(0x26,2),	//!< PINC:2/A2
    P12 = GPIO_PIN(0x26,3),	//!< PINC:3/A3
    P13 = GPIO_PIN(0x26,4),	//!< PINC:4/A4/SDA
    P14 = GPIO_PIN(0x26,5),	//!< PINC:5/A5/SCL
    P15 = 0,
    P16 = 0,

    P17 = GPIO_PIN(0x29,0),	//!< PIND:0
    P18 = GPIO_PIN(0x29,1),	//!< PIND:1
    P19 = GPIO_PIN(0x29,2),	//!< PIND:2
    P20 = GPIO_PIN(0x29,3),	//!< PIND:3
    P21 = GPIO_PIN(0x29,4),	//!< PIND:4
    P22 = GPIO_PIN(0x29,5),	//!< PIND:5
    P23 = GPIO_PIN(0x29,6),	//!< PIND:6
    P24 = GPIO_PIN(0x29,7),	//!< PIND:7
};

#elif defined(__FIRMWARE_ARCH_ATMEGA1280__)
#define POINTER_SIZE		8		// memory address size
#define F_CPU				16000000UL  // cpu@16Mhz: we have 1 cycle each 62.5ns. 16 cycles per 1us.
#define RAM					8		// ram kilobytes
#define ROM					4		// rom kilobytes
#define TIMER8				2		// 8bit timers
#define TIMER16				4		// 16bit timers
#define TIMER32				0		// 32bit timers
#define WATCHDOG			1		//
#define PIN_TOTAL			86		// total amount of I/O lines
enum pin_e {
    P1 = GPIO_PIN(0x20,0),	//!< PINA:0
    P2 = GPIO_PIN(0x20,1),	//!< PINA:1
    P3 = GPIO_PIN(0x20,2),	//!< PINA:2
    P4 = GPIO_PIN(0x20,3),	//!< PINA:3
    P5 = GPIO_PIN(0x20,4),	//!< PINA:4
    P6 = GPIO_PIN(0x20,5),	//!< PINA:5
    P7 = GPIO_PIN(0x20,6),	//!< PINA:6
    P8 = GPIO_PIN(0x20,7),	//!< PINA:7

    P9 = GPIO_PIN(0x23,0),	//!< PINB:0/SS
    P10 = GPIO_PIN(0x23,1),	//!< PINB:1/SCK/ICSP.4
    P11 = GPIO_PIN(0x23,2),	//!< PINB:2/MOSI/ICSP.3
    P12 = GPIO_PIN(0x23,3),	//!< PINB:3/MISO/ICSP.1
    P13 = GPIO_PIN(0x23,4),	//!< PINB:4
    P14 = GPIO_PIN(0x23,5),	//!< PINB:5
    P15 = GPIO_PIN(0x23,6),	//!< PINB:6
    P16 = GPIO_PIN(0x23,7),	//!< PINB:7

    P17 = GPIO_PIN(0x26,0),	//!< PINC:0
    P18 = GPIO_PIN(0x26,1),	//!< PINC:1
    P19 = GPIO_PIN(0x26,2),	//!< PINC:2
    P20 = GPIO_PIN(0x26,3),	//!< PINC:3
    P21 = GPIO_PIN(0x26,4),	//!< PINC:4
    P22 = GPIO_PIN(0x26,5),	//!< PINC:5
    P23 = GPIO_PIN(0x26,6),	//!< PINC:6
    P24 = GPIO_PIN(0x26,7),	//!< PINC:7

    P25 = GPIO_PIN(0x29,0),	//!< PIND:0
    P26 = GPIO_PIN(0x29,1),	//!< PIND:1/SDA
    P27 = GPIO_PIN(0x29,2),	//!< PIND:2/SCL
    P28 = GPIO_PIN(0x29,3),	//!< PIND:3
    P29 = 0,
    P30 = 0,
    P31 = 0,
    P32 = GPIO_PIN(0x29,7),	//!< PIND:7

    P33 = GPIO_PIN(0x2c,0),	//!< PINE:0
    P34 = GPIO_PIN(0x2c,1),	//!< PINE:1
    P35 = 0,
    P36 = GPIO_PIN(0x2c,3),	//!< PINE:3
    P37 = GPIO_PIN(0x2c,4),	//!< PINE:4
    P38 = GPIO_PIN(0x2c,5),	//!< PINE:5
    P39 = 0,
    P40 = 0,

    P41 = GPIO_PIN(0x2f,0),	//!< PINF:0/A0
    P42 = GPIO_PIN(0x2f,1),	//!< PINF:1/A1
    P43 = GPIO_PIN(0x2f,2),	//!< PINF:2/A2
    P44 = GPIO_PIN(0x2f,3),	//!< PINF:3/A3
    P45 = GPIO_PIN(0x2f,4),	//!< PINF:4/A4
    P46 = GPIO_PIN(0x2f,5),	//!< PINF:5/A5
    P47 = GPIO_PIN(0x2f,6),	//!< PINF:6/A6
    P48 = GPIO_PIN(0x2f,7),	//!< PINF:7/A7

    P49 = GPIO_PIN(0x32,0),	//!< PING:0
    P50 = GPIO_PIN(0x32,1),	//!< PING:1
    P51 = GPIO_PIN(0x32,2),	//!< PING:2
    P52 = 0,
    P53 = 0,
    P54 = GPIO_PIN(0x32,5),	//!< PING:5
    P55 = 0,
    P56 = 0,

    P57 = GPIO_PIN(0x100,0),	//!< PINH:0
    P58 = GPIO_PIN(0x100,1),	//!< PINH:1
    P59 = 0,
    P60 = GPIO_PIN(0x100,3),	//!< PINH:3
    P61 = GPIO_PIN(0x100,4),	//!< PINH:4
    P62 = GPIO_PIN(0x100,5),	//!< PINH:5
    P63 = GPIO_PIN(0x100,6),	//!< PINH:6
    P64 = 0,

    P65 = GPIO_PIN(0x106,0),	//!< PINK:0/A8
    P66 = GPIO_PIN(0x106,1),	//!< PINK:1/A9
    P67 = GPIO_PIN(0x106,2),	//!< PINK:2/A10
    P68 = GPIO_PIN(0x106,3),	//!< PINK:3/A11
    P69 = GPIO_PIN(0x106,4),	//!< PINK:4/A12
    P70 = GPIO_PIN(0x106,5),	//!< PINK:5/A13
    P71 = GPIO_PIN(0x106,6),	//!< PINK:6/A14
    P72 = GPIO_PIN(0x106,7),	//!< PINK:7/A15

    P73 = GPIO_PIN(0x103,0),	//!< PINJ:0
    P74 = GPIO_PIN(0x103,1),	//!< PINJ:1
    P75 = GPIO_PIN(0x103,2),	//!< PINJ:2
    P76 = GPIO_PIN(0x103,3),	//!< PINJ:3
    P77 = GPIO_PIN(0x103,4),	//!< PINJ:4
    P78 = GPIO_PIN(0x103,5),	//!< PINJ:5
    P79 = GPIO_PIN(0x103,6),	//!< PINJ:6
    P80 = GPIO_PIN(0x103,7),	//!< PINJ:7

    P81 = GPIO_PIN(0x109,0),	//!< PINL:0
    P82 = GPIO_PIN(0x109,1),	//!< PINL:1
    P83 = GPIO_PIN(0x109,2),	//!< PINL:2
    P84 = GPIO_PIN(0x109,3),	//!< PINL:3
    P85 = GPIO_PIN(0x109,4),	//!< PINL:4
    P86 = GPIO_PIN(0x109,5),	//!< PINL:5
    P87 = GPIO_PIN(0x109,6),	//!< PINL:6
    P88 = GPIO_PIN(0x109,7),	//!< PINL:7
};

#elif defined(__FIRMWARE_ARCH_ATMEGA2560__)
#define POINTER_SIZE		8		// memory address size
#define F_CPU				16000000UL  // cpu@16Mhz: we have 1 cycle each 62.5ns. 16 cycles per 1us.
#define RAM					8		// ram kilobytes
#define ROM					4		// rom kilobytes
#define TIMER8				2		// 8bit timers
#define TIMER16				4		// 16bit timers
#define TIMER32				0		// 32bit timers
#define WATCHDOG			1		//
#define PIN_TOTAL			86		// total amount of I/O lines
enum pin_e {
    P1 = GPIO_PIN(0x20,0),	//!< PINA:0
    P2 = GPIO_PIN(0x20,1),	//!< PINA:1
    P3 = GPIO_PIN(0x20,2),	//!< PINA:2
    P4 = GPIO_PIN(0x20,3),	//!< PINA:3
    P5 = GPIO_PIN(0x20,4),	//!< PINA:4
    P6 = GPIO_PIN(0x20,5),	//!< PINA:5
    P7 = GPIO_PIN(0x20,6),	//!< PINA:6
    P8 = GPIO_PIN(0x20,7),	//!< PINA:7

    P9 = GPIO_PIN(0x23,0),	//!< PINB:0/SS
    P10 = GPIO_PIN(0x23,1),	//!< PINB:1/SCK/ICSP.4
    P11 = GPIO_PIN(0x23,2),	//!< PINB:2/MOSI/ICSP.3
    P12 = GPIO_PIN(0x23,3),	//!< PINB:3/MISO/ICSP.1
    P13 = GPIO_PIN(0x23,4),	//!< PINB:4
    P14 = GPIO_PIN(0x23,5),	//!< PINB:5
    P15 = GPIO_PIN(0x23,6),	//!< PINB:6
    P16 = GPIO_PIN(0x23,7),	//!< PINB:7

    P17 = GPIO_PIN(0x26,0),	//!< PINC:0
    P18 = GPIO_PIN(0x26,1),	//!< PINC:1
    P19 = GPIO_PIN(0x26,2),	//!< PINC:2
    P20 = GPIO_PIN(0x26,3),	//!< PINC:3
    P21 = GPIO_PIN(0x26,4),	//!< PINC:4
    P22 = GPIO_PIN(0x26,5),	//!< PINC:5
    P23 = GPIO_PIN(0x26,6),	//!< PINC:6
    P24 = GPIO_PIN(0x26,7),	//!< PINC:7

    P25 = GPIO_PIN(0x29,0),	//!< PIND:0
    P26 = GPIO_PIN(0x29,1),	//!< PIND:1/SDA
    P27 = GPIO_PIN(0x29,2),	//!< PIND:2/SCL
    P28 = GPIO_PIN(0x29,3),	//!< PIND:3
    P29 = 0,
    P30 = 0,
    P31 = 0,
    P32 = GPIO_PIN(0x29,7),	//!< PIND:7

    P33 = GPIO_PIN(0x2c,0),	//!< PINE:0
    P34 = GPIO_PIN(0x2c,1),	//!< PINE:1
    P35 = 0,
    P36 = GPIO_PIN(0x2c,3),	//!< PINE:3
    P37 = GPIO_PIN(0x2c,4),	//!< PINE:4
    P38 = GPIO_PIN(0x2c,5),	//!< PINE:5
    P39 = 0,
    P40 = 0,

    P41 = GPIO_PIN(0x2f,0),	//!< PINF:0/A0
    P42 = GPIO_PIN(0x2f,1),	//!< PINF:1/A1
    P43 = GPIO_PIN(0x2f,2),	//!< PINF:2/A2
    P44 = GPIO_PIN(0x2f,3),	//!< PINF:3/A3
    P45 = GPIO_PIN(0x2f,4),	//!< PINF:4/A4
    P46 = GPIO_PIN(0x2f,5),	//!< PINF:5/A5
    P47 = GPIO_PIN(0x2f,6),	//!< PINF:6/A6
    P48 = GPIO_PIN(0x2f,7),	//!< PINF:7/A7

    P49 = GPIO_PIN(0x32,0),	//!< PING:0
    P50 = GPIO_PIN(0x32,1),	//!< PING:1
    P51 = GPIO_PIN(0x32,2),	//!< PING:2
    P52 = 0,
    P53 = 0,
    P54 = GPIO_PIN(0x32,5),	//!< PING:5
    P55 = 0,
    P56 = 0,

    P57 = GPIO_PIN(0x100,0),	//!< PINH:0
    P58 = GPIO_PIN(0x100,1),	//!< PINH:1
    P59 = 0,
    P60 = GPIO_PIN(0x100,3),	//!< PINH:3
    P61 = GPIO_PIN(0x100,4),	//!< PINH:4
    P62 = GPIO_PIN(0x100,5),	//!< PINH:5
    P63 = GPIO_PIN(0x100,6),	//!< PINH:6
    P64 = 0,

    P65 = GPIO_PIN(0x106,0),	//!< PINK:0/A8
    P66 = GPIO_PIN(0x106,1),	//!< PINK:1/A9
    P67 = GPIO_PIN(0x106,2),	//!< PINK:2/A10
    P68 = GPIO_PIN(0x106,3),	//!< PINK:3/A11
    P69 = GPIO_PIN(0x106,4),	//!< PINK:4/A12
    P70 = GPIO_PIN(0x106,5),	//!< PINK:5/A13
    P71 = GPIO_PIN(0x106,6),	//!< PINK:6/A14
    P72 = GPIO_PIN(0x106,7),	//!< PINK:7/A15

    P73 = GPIO_PIN(0x103,0),	//!< PINJ:0
    P74 = GPIO_PIN(0x103,1),	//!< PINJ:1
    P75 = GPIO_PIN(0x103,2),	//!< PINJ:2
    P76 = GPIO_PIN(0x103,3),	//!< PINJ:3
    P77 = GPIO_PIN(0x103,4),	//!< PINJ:4
    P78 = GPIO_PIN(0x103,5),	//!< PINJ:5
    P79 = GPIO_PIN(0x103,6),	//!< PINJ:6
    P80 = GPIO_PIN(0x103,7),	//!< PINJ:7

    P81 = GPIO_PIN(0x109,0),	//!< PINL:0
    P82 = GPIO_PIN(0x109,1),	//!< PINL:1
    P83 = GPIO_PIN(0x109,2),	//!< PINL:2
    P84 = GPIO_PIN(0x109,3),	//!< PINL:3
    P85 = GPIO_PIN(0x109,4),	//!< PINL:4
    P86 = GPIO_PIN(0x109,5),	//!< PINL:5
    P87 = GPIO_PIN(0x109,6),	//!< PINL:6
    P88 = GPIO_PIN(0x109,7),	//!< PINL:7
};

#elif defined(__FIRMWARE_ARCH_ESP8266__)
// TODO
#elif defined(__FIRMWARE_ARCH_ESP32__)
// TODO
#elif defined(__FIRMWARE_ARCH_STM32__)
// TODO
#else
#error "Please edit platform.h and add your MCU."
#endif

//
#if defined(__FIRMWARE_BOARD_HOSTLINUX__)
#define PWM_CH				0
#define PWM_MIN_RESOLUTION  0
#define PWM_MAX_RESOLUTION  0
#define ADC_CH				0
#define ONEWIRE_CH			0
#define TTY_CH				0
#define I2C_CH				0
#define SPI_CH				0
#define RX_BUFFER_SIZE		64
#define TX_BUFFER_SIZE		64
#define DEFAULT_BAUD		57600

#elif defined(__FIRMWARE_BOARD_SIMULINUX__)
#define PWM_CH				1		// pwm channels
#define PWM_MIN_RESOLUTION  8		//
#define PWM_MAX_RESOLUTION  8		//
#define ADC_CH				1		// adc channels
#define ONEWIRE_CH			PIN_TOTAL // 1wire interfaces
#define TTY_CH				2		// tty interfaces
#define I2C_CH				1		// i2c interfaces
#define SPI_CH				1		// spi interfaces
#define RX_BUFFER_SIZE		64		// 1,2,4,8,16,32,64,128 or 256 bytes
#define TX_BUFFER_SIZE		64		// 1,2,4,8,16,32,64,128 or 256 bytes
#define DEFAULT_BAUD		57600

#elif defined(__FIRMWARE_BOARD_LINUX__)
#define PWM_CH				1		// pwm channels
#define PWM_MIN_RESOLUTION  8		//
#define PWM_MAX_RESOLUTION  8		//
#define ADC_CH				1		// adc channels
#define ONEWIRE_CH			PIN_TOTAL // 1wire interfaces
#define TTY_CH				2		// tty interfaces
#define I2C_CH				1		// i2c interfaces
#define SPI_CH				1		// spi interfaces
#define RX_BUFFER_SIZE		64		// 1,2,4,8,16,32,64,128 or 256 bytes
#define TX_BUFFER_SIZE		64		// 1,2,4,8,16,32,64,128 or 256 bytes
#define DEFAULT_BAUD		57600

#elif defined(__FIRMWARE_BOARD_RPI__)
#define PWM_CH				1		// pwm channels
#define PWM_MIN_RESOLUTION  8		//
#define PWM_MAX_RESOLUTION  8		//
#define ADC_CH				1		// adc channels
#define ONEWIRE_CH			PIN_TOTAL // 1wire interfaces
#define TTY_CH				2		// tty interfaces
#define I2C_CH				1		// i2c interfaces
#define SPI_CH				1		// spi interfaces
#define RX_BUFFER_SIZE		64		// 1,2,4,8,16,32,64,128 or 256 bytes
#define TX_BUFFER_SIZE		64		// 1,2,4,8,16,32,64,128 or 256 bytes
#define DEFAULT_BAUD		57600

#elif defined(__FIRMWARE_BOARD_ODROID__)
#define PWM_CH				1		// pwm channels
#define PWM_MIN_RESOLUTION  8		//
#define PWM_MAX_RESOLUTION  8		//
#define ADC_CH				1		// adc channels
#define ONEWIRE_CH			PIN_TOTAL // 1wire interfaces
#define TTY_CH				2		// tty interfaces
#define I2C_CH				1		// i2c interfaces
#define SPI_CH				1		// spi interfaces
#define RX_BUFFER_SIZE		64		// 1,2,4,8,16,32,64,128 or 256 bytes
#define TX_BUFFER_SIZE		64		// 1,2,4,8,16,32,64,128 or 256 bytes
#define DEFAULT_BAUD		57600

#elif defined(__FIRMWARE_BOARD_GENERIC328P__)
#define PWM_CH				6
#define PWM_MIN_RESOLUTION  8
#define PWM_MAX_RESOLUTION  8
#define ADC_CH				8
#define ONEWIRE_CH			PIN_TOTAL
#define TTY_CH				1
#define I2C_CH				1
#define SPI_CH				1
#define RX_BUFFER_SIZE		64
#define TX_BUFFER_SIZE		64
#define DEFAULT_BAUD		57600

#elif defined(__FIRMWARE_BOARD_MELZI__)
#define PWM_CH				6
#define PWM_MIN_RESOLUTION  8
#define PWM_MAX_RESOLUTION  8
#define ADC_CH				8
#define ONEWIRE_CH			PIN_TOTAL
#define TTY_CH				2
#define I2C_CH				1
#define SPI_CH				1
#define RX_BUFFER_SIZE		64
#define TX_BUFFER_SIZE		64
#define DEFAULT_BAUD		57600

#elif defined(__FIRMWARE_BOARD_MKSGENL__)
#define PWM_CH				16
#define PWM_MIN_RESOLUTION  8
#define PWM_MAX_RESOLUTION  8
#define ADC_CH				16
#define ONEWIRE_CH			PIN_TOTAL
#define TTY_CH				4
#define I2C_CH				1
#define SPI_CH				1
#define RX_BUFFER_SIZE		64
#define TX_BUFFER_SIZE		64
#define DEFAULT_BAUD		57600

#else
#error "Please edit platform.h and add your board."
#endif

//
#ifndef PIN_IS_DIGITAL
#define PIN_IS_DIGITAL(p)	(0)
#endif
#ifndef PIN_IS_ANALOG
#define PIN_IS_ANALOG(p)	(0)
#endif
#ifndef PWM_MIN_RESOLUTION
#define PWM_MIN_RESOLUTION	(8)
#endif

//
#define RX_BUFFER_MASK		( RX_BUFFER_SIZE - 1 )
#define TX_BUFFER_MASK		( TX_BUFFER_SIZE - 1 )
#if ( RX_BUFFER_SIZE & RX_BUFFER_MASK )
#error "RX buffer size is not a power of 2."
#endif
#if ( TX_BUFFER_SIZE & TX_BUFFER_MASK )
#error "TX buffer size is not a power of 2."
#endif

#endif

