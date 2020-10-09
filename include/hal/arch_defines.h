
#ifndef HAL_ARCH_DEFINES_H
#define HAL_ARCH_DEFINES_H

	// Board Specific Configuration
#ifndef digitalPinHasPWM
#define digitalPinHasPWM(p)     IS_PIN_DIGITAL(p)
#endif

#if defined(digitalPinToInterrupt) && defined(NOT_AN_INTERRUPT)
#define IS_PIN_INTERRUPT(p)     (digitalPinToInterrupt(p) > NOT_AN_INTERRUPT)
#else
#define IS_PIN_INTERRUPT(p)     (0)
#endif

#if defined(__FIRMWARE_ARCH_x86__)
#define WIRE	1
#define UART0_RX 2
#define UART0_TX 3
#define SDA		4
#define SCL		5
#define SS		6
#define MOSI	7
#define MISO	8
#define SCK		9
#define LED		13
#define FIRST_ANALOG_PIN		8
#define TOTAL_ANALOG_PINS       8
#define TOTAL_PINS              16 // 8 digital + 8 analog
#define VERSION_BLINK_PIN       1
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= FIRST_ANALOG_PIN && (p) < (FIRST_ANALOG_PIN+TOTAL_ANALOG_PINS))
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_ONEWIRE(p)       ((p) == WIRE)
#define IS_PIN_UART(p)			((p) == UART0_RX || (p) == UART0_TX)
#define IS_PIN_I2C(p)           ((p) == SDA || (p) == SCL)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - FIRST_ANALOG_PIN)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
//
#define ONEWIRE_NO				0
#define UARTS_NO				1
#define I2C_NO					0
#define SPI_NO					0
#define COMMPORTS_NO			(ONEWIRE_NO+UARTS_NO+I2C_NO+SPI_NO)
#define DEFAULT_BAUD			57600
#define RX_BUFFER_SIZE			64 // 1,2,4,8,16,32,64,128 or 256 bytes
#define RX_BUFFER_MASK			( RX_BUFFER_SIZE - 1 )
#if ( RX_BUFFER_SIZE & RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#define TX_BUFFER_SIZE			64 // 1,2,4,8,16,32,64,128 or 256 bytes
#define TX_BUFFER_MASK			( TX_BUFFER_SIZE - 1 )
#if ( TX_BUFFER_SIZE & TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif

#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
	// TODO
#if defined(NUM_ANALOG_INPUTS) && NUM_ANALOG_INPUTS == 6
#define TOTAL_ANALOG_PINS       6
#define TOTAL_PINS              20 // 14 digital + 6 analog
#else
#define TOTAL_ANALOG_PINS       8
#define TOTAL_PINS              22 // 14 digital + 8 analog
#endif
#define VERSION_BLINK_PIN       13
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) <= 19)
#define IS_PIN_ANALOG(p)        ((p) >= 14 && (p) < 14 + TOTAL_ANALOG_PINS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) - 2 < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 18 || (p) == 19)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 14)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         ((p) - 2)
#define ARDUINO_PINOUT_OPTIMIZE 1

	// Sanguino
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
	// TODO
#define TOTAL_ANALOG_PINS       8
#define TOTAL_PINS              32 // 24 digital + 8 analog
#define VERSION_BLINK_PIN       0
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= 24 && (p) < TOTAL_PINS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 16 || (p) == 17)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 24)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         ((p) - 2)

	// Arduino Mega
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	// TODO
#define TOTAL_ANALOG_PINS       16
#define TOTAL_PINS              70 // 54 digital + 16 analog
#define VERSION_BLINK_PIN       13
#define PIN_SERIAL1_RX          19
#define PIN_SERIAL1_TX          18
#define PIN_SERIAL2_RX          17
#define PIN_SERIAL2_TX          16
#define PIN_SERIAL3_RX          15
#define PIN_SERIAL3_TX          14
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= 54 && (p) < TOTAL_PINS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 2 && (p) - 2 < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 20 || (p) == 21)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) > 13 && (p) < 20)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 54)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         ((p) - 2)

	// ESP8266
	// note: boot mode GPIOs 0, 2 and 15 can be used as outputs, GPIOs 6-11 are in use for flash IO
#elif defined(__FIRMWARE_ARCH_ESP8266__)
	// TODO
#define TOTAL_ANALOG_PINS       NUM_ANALOG_INPUTS
#define TOTAL_PINS              A0 + NUM_ANALOG_INPUTS
#define PIN_SERIAL_RX           3
#define PIN_SERIAL_TX           1
#define IS_PIN_DIGITAL(p)       (((p) >= 0 && (p) <= 5) || ((p) >= 12 && (p) < A0))
#define IS_PIN_ANALOG(p)        ((p) >= A0 && (p) < A0 + NUM_ANALOG_INPUTS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == SDA || (p) == SCL)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_INTERRUPT(p)     (digitalPinToInterrupt(p) > NOT_AN_INTERRUPT)
#define IS_PIN_SERIAL(p)        ((p) == PIN_SERIAL_RX || (p) == PIN_SERIAL_TX)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - A0)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)
#define DEFAULT_PWM_RESOLUTION  10

#elif defined(__FIRMWARE_ARCH_ESP32__)
	// TODO
#define TOTAL_ANALOG_PINS       NUM_ANALOG_INPUTS
#define TOTAL_PINS              A0 + NUM_ANALOG_INPUTS
#define PIN_SERIAL_RX           3
#define PIN_SERIAL_TX           1
#define IS_PIN_DIGITAL(p)       (((p) >= 0 && (p) <= 5) || ((p) >= 12 && (p) < A0))
#define IS_PIN_ANALOG(p)        ((p) >= A0 && (p) < A0 + NUM_ANALOG_INPUTS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == SDA || (p) == SCL)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_INTERRUPT(p)     (digitalPinToInterrupt(p) > NOT_AN_INTERRUPT)
#define IS_PIN_SERIAL(p)        ((p) == PIN_SERIAL_RX || (p) == PIN_SERIAL_TX)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - A0)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)
#define DEFAULT_PWM_RESOLUTION  10

	// STM32 based boards
#elif defined(__FIRMWARE_ARCH_STM32__)
	// TODO
#define TOTAL_ANALOG_PINS       NUM_ANALOG_INPUTS
#define TOTAL_PINS              NUM_DIGITAL_PINS
#define TOTAL_PORTS             MAX_NB_PORT
#define VERSION_BLINK_PIN       LED_BUILTIN
	// PIN_SERIALY_RX/TX defined in the variant.h
#define IS_PIN_DIGITAL(p)       (digitalPinIsValid(p) && !pinIsSerial(p))
#define IS_PIN_ANALOG(p)        ((p >= A0) && (p < (A0 + TOTAL_ANALOG_PINS)) && !pinIsSerial(p))
#define IS_PIN_PWM(p)           (IS_PIN_DIGITAL(p) && digitalPinHasPWM(p))
#define IS_PIN_SERVO(p)         IS_PIN_DIGITAL(p)
#define IS_PIN_I2C(p)           (IS_PIN_DIGITAL(p) && digitalPinHasI2C(p))
#define IS_PIN_SPI(p)           (IS_PIN_DIGITAL(p) && digitalPinHasSPI(p))
#define IS_PIN_INTERRUPT(p)     (IS_PIN_DIGITAL(p) && (digitalPinToInterrupt(p) > NOT_AN_INTERRUPT)))
#define IS_PIN_SERIAL(p)        (digitalPinHasSerial(p) && !pinIsSerial(p))
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        (p-A0)
#define PIN_TO_PWM(p)           (p)
#define PIN_TO_SERVO(p)         (p)
#define DEFAULT_PWM_RESOLUTION  PWM_RESOLUTION

	// anything else
#else
#error "Please edit Boards.h with a hardware abstraction for this board"
#endif

	// macros return 0 if not defined in specific board section
#ifndef IS_PIN_ONEWIRE
#define IS_PIN_ONEWIRE(p)           (0)
#endif
#ifndef IS_PIN_UART
#define IS_PIN_UART(p)        (0)
#endif
#ifndef IS_PIN_I2C
#define IS_PIN_I2C(p)           (0)
#endif
#ifndef IS_PIN_SPI
#define IS_PIN_SPI(p)           (0)
#endif
#ifndef DEFAULT_PWM_RESOLUTION
#define DEFAULT_PWM_RESOLUTION  8
#endif
#ifndef TOTAL_PORTS
#define TOTAL_PORTS             ((TOTAL_PINS + 7) / 8)
#endif

#endif // UTILITY_ARCH_DEFINES

