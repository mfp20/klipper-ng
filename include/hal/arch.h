
#ifndef HAL_ARCH_H
#define HAL_ARCH_H

extern "C" {

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h> //
#include <utility/cbuffer.h>

//
// creates a bit mask. note: the compiler will truncate the value to 16-bits max.
#define BIT(x) (0x01 << (x))
// creates a bit mask for unsigned long (32 bit). 
#define BITLONG(x) ((unsigned long)0x00000001 << (x))
// to get bit number 3 of 'foo', bit_get(foo, BIT(3))
#define BIT_GET(p,m) ((p) & (m))
// to set bit number 3 of 'foo', bit_set(foo, BIT(3))
#define BIT_SET(p,m) ((p) |= (m))
// to clear bit number 3 of 'foo', bit_clear(foo, BIT(3))
#define BIT_CLEAR(p,m) ((p) &= ~(m))
// to flip bit number 3 of 'foo', bit_flip(foo, BIT(3))
#define BIT_FLIP(p,m) ((p) ^= (m))
// to set or clear bar's bit 0, depending on foo's bit 4, bit_write(bit_get(foo, BIT(4)), bar, BIT(0))
#define BIT_WRITE(c,p,m) (c ? BIT_SET(p,m) : BIT_CLEAR(p,m))

//
#define COMMPORTS_NO 1
//
#define RX_BUFFER_SIZE 64 // 1,2,4,8,16,32,64,128 or 256 bytes
#define RX_BUFFER_MASK ( RX_BUFFER_SIZE - 1 )
#if ( RX_BUFFER_SIZE & RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#define TX_BUFFER_SIZE 64 // 1,2,4,8,16,32,64,128 or 256 bytes
#define TX_BUFFER_MASK ( TX_BUFFER_SIZE - 1 )
#if ( TX_BUFFER_SIZE & TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif
#define ERROR_NO 0 // no error
#define ERROR_FRAME 1 // Frame Error (FEn)
#define ERROR_DATAOVR 2 // Data OverRun (DORn)
#define ERROR_PARITY 3 // Parity Error (UPEn)
#define ERROR_BUFOVF 4 // Buffer overflow

//
// Board Specific Configuration
//

#ifndef digitalPinHasPWM
#define digitalPinHasPWM(p)     IS_PIN_DIGITAL(p)
#endif

#if defined(digitalPinToInterrupt) && defined(NOT_AN_INTERRUPT)
#define IS_PIN_INTERRUPT(p)     (digitalPinToInterrupt(p) > NOT_AN_INTERRUPT)
#else
#define IS_PIN_INTERRUPT(p)     (0)
#endif

// Normally Servo.h must be included before Firmata.h (which then includes
// this file).  If Servo.h wasn't included, this allows the code to still
// compile, but without support for any Servos.  Hopefully that's what the
// user intended by not including Servo.h
//#ifndef MAX_SERVOS
//#define MAX_SERVOS 0
//#endif

/*
General Constants:

These constants provide basic information Firmata requires.

TOTAL_PINS: The total number of pins Firmata implemented by Firmata.
    Usually this will match the number of pins the Arduino functions
    implement, including any pins pins capable of analog or digital.
    However, Firmata may implement any number of pins.  For example,
    on Arduino Mini with 8 analog inputs, 6 of these may be used
    for digital functions, and 2 are analog only.  On such boards,
    Firmata can implement more pins than Arduino's pinMode()
    function, in order to accommodate those special pins.  The
    Firmata protocol supports a maximum of 128 pins, so this
    constant must not exceed 128.

TOTAL_ANALOG_PINS: The total number of analog input pins implemented.
    The Firmata protocol allows up to 16 analog inputs, accessed
    using offsets 0 to 15.  Because Firmata presents the analog
    inputs using different offsets than the actual pin numbers
    (a legacy of Arduino's analogRead function, and the way the
    analog input capable pins are physically labeled on all
    Arduino boards), the total number of analog input signals
    must be specified.  16 is the maximum.

VERSION_BLINK_PIN: When Firmata starts up, it will blink the version
    number.  This constant is the Arduino pin number where a
    LED is connected.


Pin Mapping Macros:

These macros provide the mapping between pins as implemented by
Firmata protocol and the actual pin numbers used by the Arduino
functions.  Even though such mappings are often simple, pin
numbers received by Firmata protocol should always be used as
input to these macros, and the result of the macro should be
used with any Arduino function.

When Firmata is extended to support a new pin mode or feature,
a pair of macros should be added and used for all hardware
access.  For simple 1:1 mapping, these macros add no actual
overhead, yet their consistent use allows source code which
uses them consistently to be easily adapted to all other boards
with different requirements.

IS_PIN_XXXX(pin): The IS_PIN macros resolve to true or non-zero
    if a pin as implemented by Firmata corresponds to a pin
    that actually implements the named feature.

PIN_TO_XXXX(pin): The PIN_TO macros translate pin numbers as
    implemented by Firmata to the pin numbers needed as inputs
    to the Arduino functions.  The corresponding IS_PIN macro
    should always be tested before using a PIN_TO macro, so
    these macros only need to handle valid Firmata pin
    numbers for the named feature.
*/

// Linux 'board' (simulator)
#if defined(__FIRMWARE_BOARD_LINUX__)
// TODO, check values
#define FIRST_ANALOG_PIN	8
#define TOTAL_ANALOG_PINS       6
#define TOTAL_PINS              20 // 14 digital + 6 analog
#define VERSION_BLINK_PIN       1
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= FIRST_ANALOG_PIN && (p) < (FIRST_ANALOG_PIN+TOTAL_ANALOG_PINS))
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == SDA || (p) == SCL)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - FIRST_ANALOG_PIN)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)
#define DEFAULT_BAUD		57600

// Arduino Duemilanove, Diecimila, and NG
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
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

// Wiring (and board)
#elif defined(WIRING)
#define VERSION_BLINK_PIN       WLED
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= FIRST_ANALOG_PIN && (p) < (FIRST_ANALOG_PIN+TOTAL_ANALOG_PINS))
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == SDA || (p) == SCL)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - FIRST_ANALOG_PIN)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// old Arduinos
#elif defined(__AVR_ATmega8__)
#define TOTAL_ANALOG_PINS       6
#define TOTAL_PINS              20 // 14 digital + 6 analog
#define VERSION_BLINK_PIN       13
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) <= 19)
#define IS_PIN_ANALOG(p)        ((p) >= 14 && (p) <= 19)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) - 2 < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 18 || (p) == 19)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 14)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         ((p) - 2)
#define ARDUINO_PINOUT_OPTIMIZE 1

// Arduino Mega
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
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

// Arduino DUE
#elif defined(__SAM3X8E__)
#define TOTAL_ANALOG_PINS       12
#define TOTAL_PINS              66 // 54 digital + 12 analog
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
#define IS_PIN_I2C(p)           ((p) == 20 || (p) == 21) // 70 71
#define IS_PIN_SERIAL(p)        ((p) > 13 && (p) < 20)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 54)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         ((p) - 2)

// Arduino/Genuino MKR1000
#elif defined(ARDUINO_SAMD_MKR1000)
#define TOTAL_ANALOG_PINS       7
#define TOTAL_PINS              22 // 8 digital + 3 spi + 2 i2c + 2 uart + 7 analog
#define IS_PIN_DIGITAL(p)       (((p) >= 0 && (p) <= 21) && !IS_PIN_SERIAL(p))
#define IS_PIN_ANALOG(p)        ((p) >= 15 && (p) < 15 + TOTAL_ANALOG_PINS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS) // deprecated since v2.4
#define IS_PIN_I2C(p)           ((p) == 11 || (p) == 12) // SDA = 11, SCL = 12
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == PIN_SERIAL1_RX || (p) == PIN_SERIAL1_TX) //defined in variant.h  RX = 13, TX = 14
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 15)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p) // deprecated since v2.4

// Arduino MKRZero
#elif defined(ARDUINO_SAMD_MKRZERO)
#define TOTAL_ANALOG_PINS       7
#define TOTAL_PINS              34 // 8 digital + 3 spi + 2 i2c + 2 uart + 7 analog + 3 usb + 1 aref + 5 sd + 1 bottom pad + 1 led + 1 battery adc
#define IS_PIN_DIGITAL(p)       ((((p) >= 0 && (p) <= 21) || (p) == 32) && !IS_PIN_SERIAL(p))
#define IS_PIN_ANALOG(p)        (((p) >= 15 && (p) < 15 + TOTAL_ANALOG_PINS) || (p) == 33)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS) // deprecated since v2.4
#define IS_PIN_I2C(p)           ((p) == 11 || (p) == 12) // SDA = 11, SCL = 12
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == PIN_SERIAL1_RX || (p) == PIN_SERIAL1_TX) //defined in variant.h  RX = 13, TX = 14
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 15)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p) // deprecated since v2.4

// Arduino MKRFox1200
#elif defined(ARDUINO_SAMD_MKRFox1200)
#define TOTAL_ANALOG_PINS       7
#define TOTAL_PINS              33 // 8 digital + 3 spi + 2 i2c + 2 uart + 7 analog + 3 usb + 1 aref + 5 sd + 1 bottom pad + 1 battery adc
#define IS_PIN_DIGITAL(p)       (((p) >= 0 && (p) <= 21))
#define IS_PIN_ANALOG(p)        (((p) >= 15 && (p) < 15 + TOTAL_ANALOG_PINS) || (p) == 32)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS) // deprecated since v2.4
#define IS_PIN_I2C(p)           ((p) == 11 || (p) == 12) // SDA = 11, SCL = 12
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == PIN_SERIAL1_RX || (p) == PIN_SERIAL1_TX) //defined in variant.h  RX = 13, TX = 14
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 15)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p) // deprecated since v2.4

// Arduino MKR WAN 1300
#elif defined(ARDUINO_SAMD_MKRWAN1300)
#define TOTAL_ANALOG_PINS       7
#define TOTAL_PINS              33
#define IS_PIN_DIGITAL(p)       (((p) >= 0 && (p) <= 21))
#define IS_PIN_ANALOG(p)        (((p) >= 15 && (p) < 15 + TOTAL_ANALOG_PINS) || (p) == 32)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS) // deprecated since v2.4
#define IS_PIN_I2C(p)           ((p) == 11 || (p) == 12) // SDA = 11, SCL = 12
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == PIN_SERIAL1_RX || (p) == PIN_SERIAL1_TX) //defined in variant.h  RX = 13, TX = 14
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 15)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p) // deprecated since v2.4

// Arduino MKR GSM 1400
#elif defined(ARDUINO_SAMD_MKRGSM1400)
#define TOTAL_ANALOG_PINS       7
#define TOTAL_PINS              33
#define IS_PIN_DIGITAL(p)       (((p) >= 0 && (p) <= 21))
#define IS_PIN_ANALOG(p)        (((p) >= 15 && (p) < 15 + TOTAL_ANALOG_PINS) || (p) == 32)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS) // deprecated since v2.4
#define IS_PIN_I2C(p)           ((p) == 11 || (p) == 12) // SDA = 11, SCL = 12
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == PIN_SERIAL1_RX || (p) == PIN_SERIAL1_TX) //defined in variant.h  RX = 13, TX = 14
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 15)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p) // deprecated since v2.4

// Arduino Zero
// Note this will work with an Arduino Zero Pro, but not with an Arduino M0 Pro
// Arduino M0 Pro does not properly map pins to the board labeled pin numbers
#elif defined(_VARIANT_ARDUINO_ZERO_)
#define TOTAL_ANALOG_PINS       6
#define TOTAL_PINS              25 // 14 digital + 6 analog + 2 i2c + 3 spi
#define TOTAL_PORTS             3  // set when TOTAL_PINS > num digitial I/O pins
#define VERSION_BLINK_PIN       LED_BUILTIN
//#define PIN_SERIAL1_RX          0 // already defined in zero core variant.h
//#define PIN_SERIAL1_TX          1 // already defined in zero core variant.h
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) <= 19)
#define IS_PIN_ANALOG(p)        ((p) >= 14 && (p) < 14 + TOTAL_ANALOG_PINS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS) // deprecated since v2.4
#define IS_PIN_I2C(p)           ((p) == 20 || (p) == 21) // SDA = 20, SCL = 21
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK) // SS = A2
#define IS_PIN_SERIAL(p)        ((p) == 0 || (p) == 1)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 14)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p) // deprecated since v2.4

// Arduino Primo
#elif defined(ARDUINO_PRIMO)
#define TOTAL_ANALOG_PINS       6
#define TOTAL_PINS              22 //14 digital + 6 analog + 2 i2c
#define VERSION_BLINK_PIN       LED_BUILTIN
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) < 20)
#define IS_PIN_ANALOG(p)        ((p) >= 14 && (p) < 20)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS+2)
#define IS_PIN_I2C(p)           ((p) == PIN_WIRE_SDA || (p) == PIN_WIRE_SCL) // SDA = 20, SCL = 21
#define IS_PIN_SPI(p)           ((p) == SS || (p)== MOSI || (p) == MISO || (p == SCK)) // 10, 11, 12, 13
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 14)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Arduino 101
#elif defined(_VARIANT_ARDUINO_101_X_)
#define TOTAL_ANALOG_PINS       NUM_ANALOG_INPUTS
#define TOTAL_PINS              NUM_DIGITAL_PINS // 15 digital (including ATN pin) + 6 analog
#define VERSION_BLINK_PIN       LED_BUILTIN
#define PIN_SERIAL1_RX          0
#define PIN_SERIAL1_TX          1
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) <= 20)
#define IS_PIN_ANALOG(p)        ((p) >= 14 && (p) < 14 + TOTAL_ANALOG_PINS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p) // 3, 5, 6, 9
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) < MAX_SERVOS) // deprecated since v2.4
#define IS_PIN_I2C(p)           ((p) == SDA || (p) == SCL) // SDA = 18, SCL = 19
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == 0 || (p) == 1)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 14)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p) // deprecated since v2.4

// Teensy 1.0
#elif defined(__AVR_AT90USB162__)
#define TOTAL_ANALOG_PINS       0
#define TOTAL_PINS              21 // 21 digital + no analog
#define VERSION_BLINK_PIN       6
#define PIN_SERIAL1_RX          2
#define PIN_SERIAL1_TX          3
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        (0)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           (0)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == 2 || (p) == 3)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        (0)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Teensy 2.0
#elif defined(__AVR_ATmega32U4__) && defined(CORE_TEENSY)
#define TOTAL_ANALOG_PINS       12
#define TOTAL_PINS              25 // 11 digital + 12 analog
#define VERSION_BLINK_PIN       11
#define PIN_SERIAL1_RX          7
#define PIN_SERIAL1_TX          8
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= 11 && (p) <= 22)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 5 || (p) == 6)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == 7 || (p) == 8)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        (((p) < 22) ? 21 - (p) : 11)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Teensy 3.5 and 3.6
// reference: https://github.com/PaulStoffregen/cores/blob/master/teensy3/pins_arduino.h
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__)
#define TOTAL_ANALOG_PINS       27 // 3.5 has 27 and 3.6 has 25
#define TOTAL_PINS              70 // 43 digital + 21 analog-digital + 6 analog (64-69)
#define VERSION_BLINK_PIN       13
#define PIN_SERIAL1_RX          0
#define PIN_SERIAL1_TX          1
#define PIN_SERIAL2_RX          9
#define PIN_SERIAL2_TX          10
#define PIN_SERIAL3_RX          7
#define PIN_SERIAL3_TX          8
#define PIN_SERIAL4_RX          31
#define PIN_SERIAL4_TX          32
#define PIN_SERIAL5_RX          34
#define PIN_SERIAL5_TX          33
#define PIN_SERIAL6_RX          47
#define PIN_SERIAL6_TX          48
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) <= 63)
#define IS_PIN_ANALOG(p)        (((p) >= 14 && (p) <= 23) || ((p) >= 31 && (p) <= 39) || ((p) >= 49 && (p) <= 50) || ((p) >= 64 && (p) <= 69))
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 18 || (p) == 19)
#define IS_PIN_SERIAL(p)        (((p) > 6 && (p) < 11) || ((p) == 0 || (p) == 1) || ((p) > 30 && (p) < 35) || ((p) == 47 || (p) == 48))
#define PIN_TO_DIGITAL(p)       (p)
// A0-A9 = D14-D23; A12-A20 = D31-D39; A23-A24 = D49-D50; A10-A11 = D64-D65; A21-A22 = D66-D67; A25-A26 = D68-D69
#define PIN_TO_ANALOG(p)        (((p) <= 23) ? (p) - 14 : (((p) <= 39) ? (p) - 19 : (((p) <= 50) ? (p) - 26 : (((p) <= 65) ? (p) - 55 : (((p) <= 67) ? (p) - 45 : (p) - 43)))))
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Teensy 3.0, 3.1 and 3.2
#elif defined(__MK20DX128__) || defined(__MK20DX256__)
#define TOTAL_ANALOG_PINS       14
#define TOTAL_PINS              38 // 24 digital + 10 analog-digital + 4 analog
#define VERSION_BLINK_PIN       13
#define PIN_SERIAL1_RX          0
#define PIN_SERIAL1_TX          1
#define PIN_SERIAL2_RX          9
#define PIN_SERIAL2_TX          10
#define PIN_SERIAL3_RX          7
#define PIN_SERIAL3_TX          8
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) <= 33)
#define IS_PIN_ANALOG(p)        (((p) >= 14 && (p) <= 23) || ((p) >= 34 && (p) <= 38))
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 18 || (p) == 19)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        (((p) > 6 && (p) < 11) || ((p) == 0 || (p) == 1))
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        (((p) <= 23) ? (p) - 14 : (p) - 24)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Teensy-LC
#elif defined(__MKL26Z64__)
#define TOTAL_ANALOG_PINS       13
#define TOTAL_PINS              27 // 27 digital + 13 analog-digital
#define VERSION_BLINK_PIN       13
#define PIN_SERIAL1_RX          0
#define PIN_SERIAL1_TX          1
#define PIN_SERIAL2_RX          9
#define PIN_SERIAL2_TX          10
#define PIN_SERIAL3_RX          7
#define PIN_SERIAL3_TX          8
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) <= 26)
#define IS_PIN_ANALOG(p)        ((p) >= 14)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 18 || (p) == 19)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        (((p) > 6 && (p) < 11) || ((p) == 0 || (p) == 1))
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 14)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Teensy++ 1.0 and 2.0
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
#define TOTAL_ANALOG_PINS       8
#define TOTAL_PINS              46 // 38 digital + 8 analog
#define VERSION_BLINK_PIN       6
#define PIN_SERIAL1_RX          2
#define PIN_SERIAL1_TX          3
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= 38 && (p) < TOTAL_PINS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 0 || (p) == 1)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == 2 || (p) == 3)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 38)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Leonardo
#elif defined(__AVR_ATmega32U4__)
#define TOTAL_ANALOG_PINS       12
#define TOTAL_PINS              30 // 14 digital + 12 analog + 4 SPI (D14-D17 on ISP header)
#define VERSION_BLINK_PIN       13
#define PIN_SERIAL1_RX          0
#define PIN_SERIAL1_TX          1
#define IS_PIN_DIGITAL(p)       ((p) >= 0 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= 18 && (p) < TOTAL_PINS)
#define IS_PIN_PWM(p)           ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11 || (p) == 13)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 2 || (p) == 3)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == 0 || (p) == 1)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        (p) - 18
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Intel Galileo Board (gen 1 and 2) and Intel Edison
#elif defined(ARDUINO_LINUX)
#define TOTAL_ANALOG_PINS       6
#define TOTAL_PINS              20 // 14 digital + 6 analog
#define VERSION_BLINK_PIN       13
#define PIN_SERIAL1_RX          0
#define PIN_SERIAL1_TX          1
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) <= 19)
#define IS_PIN_ANALOG(p)        ((p) >= 14 && (p) <= 19)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         (IS_PIN_DIGITAL(p) && (p) - 2 < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == SDA || (p) == SCL)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == 0 || (p) == 1)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 14)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         ((p) - 2)

// RedBearLab BLE Nano with factory switch settings (S1 - S10)
#elif defined(BLE_NANO)
#define TOTAL_ANALOG_PINS       6
#define TOTAL_PINS              15 // 9 digital + 3 analog
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) <= 14)
#define IS_PIN_ANALOG(p)        ((p) == 8 || (p) == 9 || (p) == 10 || (p) == 11 || (p) == 12 || (p) == 14) //A0~A5
#define IS_PIN_PWM(p)           ((p) == 3 || (p) == 5 || (p) == 6)
#define IS_PIN_SERVO(p)         ((p) >= 2 && (p) <= 7)
#define IS_PIN_I2C(p)           ((p) == SDA || (p) == SCL)
#define IS_PIN_SPI(p)           ((p) == CS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 8)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         (p)

// Pinoccio Scout
// Note: digital pins 9-16 are usable but not labeled on the board numerically.
// SS=9, MOSI=10, MISO=11, SCK=12, RX1=13, TX1=14, SCL=15, SDA=16
#elif defined(ARDUINO_PINOCCIO)
#define TOTAL_ANALOG_PINS       8
#define TOTAL_PINS              NUM_DIGITAL_PINS // 32
#define VERSION_BLINK_PIN       23
#define PIN_SERIAL1_RX          13
#define PIN_SERIAL1_TX          14
#define IS_PIN_DIGITAL(p)       (((p) >= 2) && ((p) <= 16)) || (((p) >= 24) && ((p) <= 31))
#define IS_PIN_ANALOG(p)        ((p) >= 24 && (p) <= 31)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         IS_PIN_DIGITAL(p)
#define IS_PIN_I2C(p)           ((p) == SCL || (p) == SDA)
#define IS_PIN_SPI(p)           ((p) == SS || (p) == MOSI || (p) == MISO || (p) == SCK)
#define IS_PIN_SERIAL(p)        ((p) == 13 || (p) == 14)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 24)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         ((p) - 2)

// Sanguino
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
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

// Illuminato
#elif defined(__AVR_ATmega645__)
#define TOTAL_ANALOG_PINS       6
#define TOTAL_PINS              42 // 36 digital + 6 analog
#define VERSION_BLINK_PIN       13
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) >= 36 && (p) < TOTAL_PINS)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         ((p) >= 0 && (p) < MAX_SERVOS)
#define IS_PIN_I2C(p)           ((p) == 4 || (p) == 5)
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ((p) - 36)
#define PIN_TO_PWM(p)           PIN_TO_DIGITAL(p)
#define PIN_TO_SERVO(p)         ((p) - 2)

// ESP8266
// note: boot mode GPIOs 0, 2 and 15 can be used as outputs, GPIOs 6-11 are in use for flash IO
#elif defined(ESP8266)
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
#elif defined(ARDUINO_ARCH_STM32)
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


// Adafruit Bluefruit nRF52 boards
#elif defined(ARDUINO_NRF52_ADAFRUIT)
#define TOTAL_ANALOG_PINS       NUM_ANALOG_INPUTS
#define TOTAL_PINS              32
#define VERSION_BLINK_PIN       LED_BUILTIN
#define IS_PIN_DIGITAL(p)       ((p) >= 2 && (p) < TOTAL_PINS)
#define IS_PIN_ANALOG(p)        ((p) == PIN_A0 || (p) == PIN_A1 || (p) == PIN_A2  || (p) == PIN_A3 || \
                                 (p) == PIN_A4 || (p) == PIN_A5 || (p) == PIN_A6  || (p) == PIN_A7)
#define IS_PIN_PWM(p)           digitalPinHasPWM(p)
#define IS_PIN_SERVO(p)         IS_PIN_DIGITAL(p)
#define IS_PIN_I2C(p)           ((p) == PIN_WIRE_SDA || (p) == PIN_WIRE_SCL)
#define IS_PIN_SPI(p)           ((p) == SS || (p)== MOSI || (p) == MISO || (p == SCK))
#define PIN_TO_DIGITAL(p)       (p)
#define PIN_TO_ANALOG(p)        ( ((p) == PIN_A0) ? 0 : ((p) == PIN_A1) ? 1 : ((p) == PIN_A2) ? 2 : ((p) == PIN_A3) ? 3 : \
                                  ((p) == PIN_A4) ? 4 : ((p) == PIN_A5) ? 5 : ((p) == PIN_A6) ? 6 : ((p) == PIN_A7) ? 7 : (127))
#define PIN_TO_PWM(p)           (p)
#define PIN_TO_SERVO(p)         (p)

// anything else
#else
#error "Please edit Boards.h with a hardware abstraction for this board"
#endif

// as long this is not defined for all boards:
#ifndef IS_PIN_SPI
#define IS_PIN_SPI(p)           (0)
#endif

#ifndef IS_PIN_SERIAL
#define IS_PIN_SERIAL(p)        0
#endif

#ifndef DEFAULT_PWM_RESOLUTION
#define DEFAULT_PWM_RESOLUTION  8
#endif

#ifndef TOTAL_PORTS
#define TOTAL_PORTS             ((TOTAL_PINS + 7) / 8)
#endif


//
// TIME KEEPING
//

// time since boot
extern volatile uint16_t tnano;
extern volatile uint16_t tmicro;
extern volatile uint16_t tmilli;
extern volatile uint16_t tsecond;

uint16_t nanos(void);
uint16_t micros(void); 
uint16_t millis(void); 
uint16_t seconds(void);


//
// PORTs/PINs GET/SET
//

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

/*
Port Access Inline Funtions:

For efficiency, Firmata protocol provides access to digital
input and output pins grouped by 8 bit ports.  When these
groups of 8 correspond to actual 8 bit ports as implemented
by the hardware, these inline functions can provide high
speed direct port access.  Otherwise, a default implementation
using 8 calls to digitalWrite or digitalRead is used.

When porting Firmata to a new board, it is recommended to
use the default functions first and focus only on the constants
and macros above.  When those are working, if optimized port
access is desired, these inline functions may be extended.
The recommended approach defines a symbol indicating which
optimization to use, and then conditional complication is
used within these functions.

readPort(port, bitmask):  Read an 8 bit port, returning the value.
   port:    The port number, Firmata pins port*8 to port*8+7
   bitmask: The actual pins to read, indicated by 1 bits.

writePort(port, value, bitmask):  Write an 8 bit port.
   port:    The port number, Firmata pins port*8 to port*8+7
   value:   The 8 bit value to write
   bitmask: The actual pins to write, indicated by 1 bits.
*/

// readPort() - Read an 8 bit port
static inline unsigned char readPort(uint8_t, uint8_t) __attribute__((always_inline, unused));
static inline unsigned char readPort(uint8_t port, uint8_t bitmask) {
  unsigned char out = 0, pin = port * 8;
  if (IS_PIN_DIGITAL(pin + 0) && (bitmask & 0x01) && pin_read(PIN_TO_DIGITAL(pin + 0))) out |= 0x01;
  if (IS_PIN_DIGITAL(pin + 1) && (bitmask & 0x02) && pin_read(PIN_TO_DIGITAL(pin + 1))) out |= 0x02;
  if (IS_PIN_DIGITAL(pin + 2) && (bitmask & 0x04) && pin_read(PIN_TO_DIGITAL(pin + 2))) out |= 0x04;
  if (IS_PIN_DIGITAL(pin + 3) && (bitmask & 0x08) && pin_read(PIN_TO_DIGITAL(pin + 3))) out |= 0x08;
  if (IS_PIN_DIGITAL(pin + 4) && (bitmask & 0x10) && pin_read(PIN_TO_DIGITAL(pin + 4))) out |= 0x10;
  if (IS_PIN_DIGITAL(pin + 5) && (bitmask & 0x20) && pin_read(PIN_TO_DIGITAL(pin + 5))) out |= 0x20;
  if (IS_PIN_DIGITAL(pin + 6) && (bitmask & 0x40) && pin_read(PIN_TO_DIGITAL(pin + 6))) out |= 0x40;
  if (IS_PIN_DIGITAL(pin + 7) && (bitmask & 0x80) && pin_read(PIN_TO_DIGITAL(pin + 7))) out |= 0x80;
  return out;
}

// writePort() - Write an 8 bit port, only touch pins specified by a bitmask
static inline unsigned char writePort(uint8_t, uint8_t, uint8_t) __attribute__((always_inline, unused));
static inline unsigned char writePort(uint8_t port, uint8_t value, uint8_t bitmask) {
  uint8_t pin = port * 8;
  if ((bitmask & 0x01)) pin_write(PIN_TO_DIGITAL(pin + 0), (value & 0x01));
  if ((bitmask & 0x02)) pin_write(PIN_TO_DIGITAL(pin + 1), (value & 0x02));
  if ((bitmask & 0x04)) pin_write(PIN_TO_DIGITAL(pin + 2), (value & 0x04));
  if ((bitmask & 0x08)) pin_write(PIN_TO_DIGITAL(pin + 3), (value & 0x08));
  if ((bitmask & 0x10)) pin_write(PIN_TO_DIGITAL(pin + 4), (value & 0x10));
  if ((bitmask & 0x20)) pin_write(PIN_TO_DIGITAL(pin + 5), (value & 0x20));
  if ((bitmask & 0x40)) pin_write(PIN_TO_DIGITAL(pin + 6), (value & 0x40));
  if ((bitmask & 0x80)) pin_write(PIN_TO_DIGITAL(pin + 7), (value & 0x80));
  return 1;
}


//
// ADC
//

extern volatile uint8_t adc_enable[16];
extern volatile uint8_t adc_value[16];
extern volatile uint8_t adc_current;


//
// PULSING
//

// 1ms pulsing frame
// 	- 8 max pulsing pins (each pulse element is a 8 bit value, each bit represent 1 pin value)
// 	- 250 max pulses per millisecond (ie: 1 each 4us)
typedef struct pin_frame_s {
	volatile uint16_t milli;
	volatile uint8_t *pulse;
	volatile uint8_t pulse_counter;
	volatile struct pin_frame_s *next;
} pin_frame_t;
// pulsing pins&frames
typedef struct pin_pulsing_s {
	volatile uint8_t pin[8];
	volatile pin_frame_t *head;
	volatile pin_frame_t *tail;
} pin_pulsing_t;
// pulsing buffer
extern volatile pin_pulsing_t pulsing;

// Detect a single pulse and return its width in micro-seconds.
uint16_t pin_pulse_detect(uint8_t pin);
// Generate a single pulse with given width in micro-seconds.
void pin_pulse_single(uint8_t pin, uint16_t width);
// Add 1 frame at 'milli(seconds)' to pulsing queue
volatile pin_frame_t* pin_pulse_multi(uint16_t milli);


//
// COMMS
//

typedef struct commport_s commport_t;
struct commport_s {
	uint8_t fd;
	cbuffer_t rx_buf;
	uint8_t rx_err;
	cbuffer_t tx_buf;
	uint8_t tx_err;
	bool (*need_rx)(commport_t *uart);
	uint8_t (*rx_byte)(commport_t *uart);
	uint8_t (*rx_all)(commport_t *uart, uint16_t timeout);
	bool (*need_tx)(commport_t *uart);
	uint8_t (*tx_byte)(commport_t *uart);
	uint8_t (*tx_all)(commport_t *uart, uint16_t timeout);
};
extern commport_t *commports[8];

// UART
// setup device and return the FD
uint8_t uart_enable(commport_t *uart, uint32_t baud);
// return 1 if the rx buffer is empty
bool uart_need_rx(commport_t *uart);
// read 1 byte from device and fill the buffer
uint8_t uart_rx_byte(commport_t *uart);
// read all available bytes from device and fill the buffer
uint8_t uart_rx_all(commport_t *uart, uint16_t timeout);
// return 1 if the tx buffer is full
bool uart_need_tx(commport_t *uart);
// write 1 byte from buffer to device
uint8_t uart_tx_byte(commport_t *uart);
// read all available bytes from buffer to device
uint8_t uart_tx_all(commport_t *uart, uint16_t timeout);
// close the device
void uart_disable(commport_t *uart);

// I2C
uint8_t i2c_enable(uint32_t baud);
bool i2c_need_rx(void);
uint8_t i2c_rx_byte(void);
uint8_t i2c_rx_all(void);
bool i2c_need_tx(void);
uint8_t i2c_tx_byte(void);
uint8_t i2c_tx_all(void);
void i2c_disable(void);

// SPI
uint8_t spi_enable(uint32_t baud);
bool spi_need_rx(void);
uint8_t spi_rx_byte(void);
uint8_t spi_rx_all(void);
bool spi_need_tx(void);
uint8_t spi_tx_byte(void);
uint8_t spi_tx_all(void);
void spi_disable(void);


//
// ARCH
//

void logger(const char *string);
void arch_init(void);
uint8_t arch_run(uint16_t snow, uint16_t mnow, uint16_t utimeout);
uint8_t arch_close(void);

}

#endif

