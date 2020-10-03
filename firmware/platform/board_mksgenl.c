
#include "board.h"

// LED
#define LED PB7
// UART0
#define RXD0 PE0
#define TXD0 PE1

// X axis
#define X_EN3 PD7
#define X_STEP3 PF0
#define X_DIR3 PF1
#define X_MAX PE4
#define X_MIN PE5
#define X_RX PK1
#define X_TX PG1
// Y axis
#define Y_EN4 PF2
#define Y_STEP4 PF6
#define Y_DIR4 PF7
#define Y_MAX PJ0 // alt: RXD3
#define Y_MIN PJ1 // alt: TXD3
#define Y_RX PK2
#define Y_TX PF5
// Z axis
#define Z_EN5 PK0
#define Z_STEP5 PL3
#define Z_DIR5 PL1
#define Z_MAX PD2 // alt: RXD1
#define Z_MIN PD3 // alt: TXD1
#define Z_RX PK3
#define Z_TX PL7
// E0
#define E0_EN1 PA2
#define E0_STEP1 PA4
#define E0_DIR1 PA6
#define E0_RX PK4
#define E0_TX PL5
// E1
#define E1_EN2 PC7
#define E1_STEP2 PC1
#define E1_DIR2 PC3
#define E1_RX PD0
#define E1_TX PD1

// HEAT
#define HEATER_E0 PB4
#define TH1 PK5 // sensor
#define HEATER_E1 PH4
#define TH2 PK7 // sensor
#define HEATER_BED PH5
#define THB PK6 // sensor
#define HEATER_FAN PH6

// EXP1
#define BEEPER PC0
#define BTN_ENC PC2
#define LCD_EN PH0 // alt: RXD2
#define LCD_RS PH1 // alt: TXD2
#define LCD_D4 PA1
#define LCD_D5 PA3
#define LCD_D6 PA5
#define LCD_D7 PA7
//
#define LCD_EN0 PE6
#define LCD_RS0 PE7

// EXP2
#define MISO PB3
#define SCK PB1
#define BTN_EN1 PC6
#define SS_SD PB0
#define BTN_EN2 PC4
#define MOSI PB2
#define SD_DET PL0
#define RESET 0
#define KILL PG0

// J24
#define D11 PB5
#define D12 PB6

// J25
#define D4 PG5
#define A4 PF4
#define D5 PE3
#define A3 PF3
#define D43 PL6
#define D6 PH3
#define D47 PL2
#define D45 PL4
#define D32 PC5
#define D39 PG2

// NOT CONNECTED
#define NONE1 PA0
#define NONE2 PD4
#define NONE3 PD5
#define NONE4 PD6
#define NONE5 PE2
#define NONE6 PG3
#define NONE7 PG4
#define NONE8 PH2
#define NONE9 PH7
#define NONE10 PJ2
#define NONE11 PJ3
#define NONE12 PJ4
#define NONE13 PJ5
#define NONE14 PJ6
#define NONE15 PJ7

void board_init(void) {
	pin_t pin_pulsing[8];
	pin_pulsing[0] = X_STEP3;
	pin_pulsing[1] = Y_STEP4;
	pin_pulsing[2] = Z_STEP5;
	pin_pulsing[3] = E0_STEP1;
	pin_pulsing[4] = E1_STEP2;
	pin_pulsing[5] = 0;
	pin_pulsing[6] = 0;
	pin_pulsing[7] = 0;
	arch_init();
}

uint8_t board_run(void) {
	return arch_run();
}

