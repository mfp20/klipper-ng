#ifndef __PROTOCOL_H
#define __PROTOCOL_H

/* message bytes
00000000 00000000 00000000 ... 00000000 00000000 00000000
|      | |      | |      | | | |      | |      | |      |
|      | |      | |      | | | |      | |      | |______|___SYNC (ex: 0x7E)
|      | |      | |      | | | |      | |______|____________CRC16-2
|      | |      | |      | | | |______|_____________________CRC16-1
|      | |      | |      | |_|______________________________Rest of payload (args)
|      | |      | |______|__________________________________1st payload byte (command handler)
|      | |______|___________________________________________Message sequence number
|______|____________________________________________________COBS code
*/
#define MSG_MAX 256
#define MSG_RX_BUFFER_SIZE (MSG_MAX*2)
#define MSG_TX_BUFFER_SIZE (MSG_MAX+(MSG_MAX/2))
#define MSG_COBS_OVERHEAD ((MSG_MAX/254)+1)
#define MSG_OVERHEAD 3
#define MSG_MIN (MSG_COBS_OVERHEAD + MSG_OVERHEAD)
#define MSG_HEADER_SIZE  2
#define MSG_PAYLOAD_MAX_SIZE (MSG_MAX - MSG_MIN)
#define MSG_TRAILER_SIZE 3
#define MSG_POS_COBS_CODE 0
#define MSG_POS_SEQ 1
#define MSG_POS_PAYLOAD 2
#define MSG_TRAILER_CRC  3
#define MSG_TRAILER_SYNC 1
#define MSG_SEQ_MASK 0x0f
#define MSG_DEST 0x10
#define MSG_SYNC 0x7E

#define TYPE_UNKNOWN 0
#define TYPE_NOOP 1
#define TYPE_START 2
#define TYPE_ACK 3
#define TYPE_NACK 4
#define TYPE_SHUTDOWN_NOW 5
#define TYPE_SHUTDOWN_LAST 6
#define TYPE_BASE_IDENTIFY 7
#define TYPE_BASE_ALLOCATE_OIDS 8
#define TYPE_BASE_GET_CONFIG 9
#define TYPE_BASE_FINALIZE_CONFIG 10
#define TYPE_BASE_GET_CLOCK 11
#define TYPE_BASE_GET_UPTIME 12
#define TYPE_BASE_EMERGENCY_STOP 13
#define TYPE_BASE_CLEAR_SHUTDOWN 14
#define TYPE_BASE_STATS 15
#define TYPE_BASE_RESET 16
#define TYPE_DEBUG_START_GROUP 17
#define TYPE_DEBUG_END_GROUP 18
#define TYPE_DEBUG_NOP 19
#define TYPE_DEBUG_PING 20
#define TYPE_DEBUG_READ 21
#define TYPE_DEBUG_WRITE 22
#define TYPE_GPIO_CONFIG 23
#define TYPE_GPIO_SCHEDULE 24
#define TYPE_GPIO_UPDATE 25
#define TYPE_GPIO_SET 26
#define TYPE_GPIO_SOFTPWM_CONFIG 27
#define TYPE_GPIO_SOFTPWM_SCHEDULE 28
#define TYPE_PWM_CONFIG 29
#define TYPE_PWM_SCHEDULE 30
#define TYPE_PWM_SET 31
#define TYPE_ADC_CONFIG 32
#define TYPE_ADC_QUERY 33
#define TYPE_ADC_STATE 34
#define TYPE_I2C_CONFIG 35
#define TYPE_I2C_MODBITS 36
#define TYPE_I2C_READ 37
#define TYPE_I2C_WRITE 38
#define TYPE_SPI_CONFIG 39
#define TYPE_SPI_CONFIG_NOCS 40
#define TYPE_SPI_SET 41
#define TYPE_SPI_TRANSFER 42
#define TYPE_SPI_SEND 43
#define TYPE_SPI_SHUTDOWN 44
#define TYPE_NO 45 // number of existing commands

#define ERROR_UNKNOWN 0
#define ERROR_GENERIC 1
#define ERROR_ALREADY_CLEARED 2
#define ERROR_CLOSE_TIMER 3
#define ERROR_SENTINEL_TIMER 4
#define ERROR_INVALID_CMD 5
#define ERROR_MSG_ENC 6
#define ERROR_CMD_PARSE 7
#define ERROR_CMD_REQ 8
#define ERROR_NOT_SHUTDOWN 9
#define ERROR_OIDS_ALREADY_ALLOC 10
#define ERROR_OID_CANT_ASSIGN 11
#define ERROR_OID_INVALID 12
#define ERROR_ALREADY_FINAL 13
#define ERROR_INVALID_MOVE_SIZE 14
#define ERROR_MOVE_Q_EMPTY 15
#define ERROR_CHUNKS_FAIL 16
#define ERROR_CHUNK_FAIL 17
#define ERROR_MISSED_EVENT 18
#define ERROR_NEXT_PWM_EXTENDS 19
#define ERROR_MISSED_PWM_EVENT 20
#define ERROR_MISSED_PIN_EVENT 21
#define ERROR_STEPPER_ACTIVE_CANT_RESET 22
#define ERROR_INVALID_COUNT_PARAM 23
#define ERROR_STEPPER_TOO_OLD 24
#define ERROR_NO_NEXT_STEP 25
#define ERROR_PAST_MAX_COUNT 26
#define ERROR_ADC_OUT_OF_RANGE 27
#define ERROR_SPI_CONFIG_INVALID 28
#define ERROR_TCOUPLE_READ_FAIL 29
#define ERROR_TCOUPLE_ADC_OOR 30
#define ERROR_TCOUPLE_INVALID 31
#define ERROR_I2C_MODIFY_BITS 32
#define ERROR_PWM_MISSED_EVENT 33
#define ERROR_BUTTONS_RETR_INVALID 34
#define ERROR_BUTTON_PAST_MAX_COUNT 35
#define ERROR_BUTTONS_MAX 36
#define ERROR_TMCUART_LARGE_DATA 37
#define ERROR_TIMER_RESCHED_PAST 38
#define ERROR_PIN_IN_INVALID 39
#define ERROR_PIN_OUT_INVALID 40
#define ERROR_PIN_ADC_INVALID 41
#define ERROR_SPI_SETUP_INVALID 42
#define ERROR_I2C_START_FAIL 43
#define ERROR_I2C_TIMEOUT 44
#define ERROR_I2C_BUS_INVALID 45
#define ERROR_PWM_ALREADY_PROG 46
#define ERROR_TIMER1_NOT_PWM 47
#define ERROR_PIN_PWM_INVALID 48
#define ERROR_WATCHDOG_EXPIRE 49
#define ERROR_NO 50

#endif // protocol.h

