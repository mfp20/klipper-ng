#ifndef HAL_DEFINES_H
#define HAL_DEFINES_H

#ifdef __GIT_REVPARSE__
#define RELEASE_DEFINES __GIT_REVPARSE__
#else
#error "Please add __GITREVPARSE__ define to your compiler command line."
#endif

#include <utility/macros.h>

//
// MCUs
//

// TODO pin numbers are random, need to check on datasheet

#if defined(__FIRMWARE_ARCH_BOGUS__)
#define TOTAL_PINS              16 // 8 digital + 8 analog
#define FIRST_ANALOG_PIN		8
#define TOTAL_ANALOG_PINS       8
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= FIRST_ANALOG_PIN && (p) < (FIRST_ANALOG_PIN+TOTAL_ANALOG_PINS))
//#define IS_PIN_INTERRUPT(p)     ((p) == 4 || (p) == 6)
//#define IS_PIN_PWM(p)           ((p) == 5 || (p) == 7)
//#define IS_PIN_1WIRE(p)			((p) == 9)
//#define IS_PIN_UART(p)			((p) == 1 || (p) == 2)
//#define IS_PIN_I2C(p)           ((p) == 3 || (p) == 4)
//#define IS_PIN_SPI(p)           ((p) == 5 || (p) == 6 || (p) == 7 || (p) == 8)
#define ONEWIRE_QTY				0
#define UARTS_QTY				1
#define I2C_QTY					0
#define SPI_QTY					0
#define COMMPORT_QTY			((ONEWIRE_QTY)+(UARTS_QTY)+(I2C_QTY)+(SPI_QTY))
//#define RX_BUFFER_SIZE			64 // 1,2,4,8,16,32,64,128 or 256 bytes
//#define TX_BUFFER_SIZE			64 // 1,2,4,8,16,32,64,128 or 256 bytes

#elif defined(__FIRMWARE_ARCH_X86__)
#define TOTAL_PINS              0

#elif defined(__FIRMWARE_ARCH_ATMEGA168__) || defined(__FIRMWARE_ARCH_ATMEGA328__) || defined(__FIRMWARE_ARCH_ATMEGA328P__)

#elif defined(__FIRMWARE_ARCH_ATMEGA644__) || defined(__FIRMWARE_ARCH_ATMEGA644P__) || defined(__FIRMWARE_ARCH_ATMEGA1284P__)

#elif defined(__FIRMWARE_ARCH_ATMEGA1280__) || defined(__FIRMWARE_ARCH_ATMEGA2560__)

#elif defined(__FIRMWARE_ARCH_ESP8266__)

#elif defined(__FIRMWARE_ARCH_ESP32__)

#elif defined(__FIRMWARE_ARCH_STM32__)

	// anything else
#else
#error "Please edit defines.h with a hardware abstraction for this mcu."
#endif

//
// BOARDs
//

#if defined(__FIRMWARE_BOARD_HOSTLINUX__)
#define PIN_1WIRE		1
#define PIN_UART0_RX	2
#define PIN_UART0_TX	3
#define PIN_SDA			4
#define PIN_SCL			5
#define PIN_SS			6
#define PIN_MOSI		7
#define PIN_MISO		8
#define PIN_SCK			9
#define PIN_LED			13

#elif defined(__FIRMWARE_BOARD_SIMULINUX__)
#define PIN_1WIRE		1
#define PIN_UART0_RX	2
#define PIN_UART0_TX	3
#define PIN_SDA			4
#define PIN_SCL			5
#define PIN_SS			6
#define PIN_MOSI		7
#define PIN_MISO		8
#define PIN_SCK			9
#define PIN_LED			13

#elif defined(__FIRMWARE_BOARD_GENERIC328P__)
	// TODO

#elif defined(__FIRMWARE_BOARD_MELZI__)
	// TODO

#elif defined(__FIRMWARE_BOARD_MKSGENL__)
	// TODO

#else
#error "Please edit defines.h with a hardware abstraction for this board."
#endif

//
// MISC
//

//
#ifndef IS_PIN_DIGITAL
#define IS_PIN_DIGITAL(p)       (0)
#endif
#ifndef IS_PIN_ANALOG
#define IS_PIN_ANALOG(p)        (0)
#endif
#ifndef IS_PIN_INTERRUPT
#define IS_PIN_INTERRUPT(p) (0)
#endif
#ifndef IS_PIN_PWM
#define IS_PIN_PWM(p) (0)
#endif
#ifndef IS_PIN_1WIRE
#define IS_PIN_1WIRE(p)   (0)
#endif
#ifndef IS_PIN_UART
#define IS_PIN_UART(p)      (0)
#endif
#ifndef IS_PIN_I2C
#define IS_PIN_I2C(p)       (0)
#endif
#ifndef IS_PIN_SPI
#define IS_PIN_SPI(p)       (0)
#endif
#ifndef DEFAULT_PWM_RESOLUTION
#define DEFAULT_PWM_RESOLUTION  8
#endif
#ifndef TOTAL_PORTS
#define TOTAL_PORTS             ((TOTAL_PINS + 7) / 8)
#endif

// pin modes
#define PIN_MODE_DIGITAL_INPUT	0x00 //
#define PIN_MODE_DIGITAL_OUTPUT	0x01 //
#define PIN_MODE_ANALOG_INPUT	0x02 // analog pin in analogInput mode
#define PIN_MODE_PWM            0x03 // digital pin in PWM output mode
#define PIN_MODE_SERVO          0x04 // digital pin in Servo output mode
#define PIN_MODE_SHIFT          0x05 // shiftIn/shiftOut mode
#define PIN_MODE_I2C            0x06 // pin included in I2C setup
#define PIN_MODE_1WIRE			0x07 // pin configured for 1-wire
#define PIN_MODE_STEPPER        0x08 // pin configured for stepper motor
#define PIN_MODE_ENCODER        0x09 // pin configured for rotary encoders
#define PIN_MODE_SERIAL         0x0A // pin configured for serial communication
#define PIN_MODE_PULLUP         0x0B // enable internal pull-up resistor for pin
#define PIN_MODE_SPI			0x0C // pin included in SPI setup
#define PIN_MODE_IGNORE         0x7F // pin configured to be ignored
#define TOTAL_PIN_MODES         14

// comm port generic
#define RX_BUFFER_MASK			( RX_BUFFER_SIZE - 1 )
#if ( RX_BUFFER_SIZE & RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#define TX_BUFFER_MASK			( TX_BUFFER_SIZE - 1 )
#if ( TX_BUFFER_SIZE & TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif
#define DEFAULT_BAUD			57600

// comm port types
#define COMMPORT_TYPE_PIN	0
#define COMMPORT_TYPE_1WIRE 1
#define COMMPORT_TYPE_UART	2
#define COMMPORT_TYPE_I2C	3
#define COMMPORT_TYPE_SPI	4

// comm port errors
#define COMMPORT_ERROR_NO				0 // no error
#define COMMPORT_ERROR_FRAME			1 // Frame Error (FEn)
#define COMMPORT_ERROR_DATAOVR			2 // Data OverRun (DORn)
#define COMMPORT_ERROR_PARITY			3 // Parity Error (UPEn)
#define COMMPORT_ERROR_BUFOVF			4 // Buffer overflow

#define MSB(a) \
	({ __typeof__ (a) _a = (a); \
	_a >> 8; })
#define LSB(a) \
	({ __typeof__ (a) _a = (a); \
	_a & 0x00FF; })

#endif // HAL_DEFINES

