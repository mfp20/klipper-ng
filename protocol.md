General protocol description is in include/protocol.h file.

Here a few tables taken from Firmata repo and adapted to help developers in visualizing the protocol in a glance.
Note: the protocol isn't finalized yet, there might be inaccuracies.

##### Table of Contents  
[1. Control bytes (0x80-0xFF)](#item1)<br>
[2. Sysex types (and modifiers)](#item2)<br>
[3. Simple event format](#item3)<br>
[4. Sysex event format](#item4)<br>
[5. Sysex data format](#item5)<br>
[5.1. Sub: SYSEX_PREFERRED_PINS_DATA format](#item6)<br>
[5.2. Sub: SYSEX_PINGROUPS_DATA format](#item7)<br>
[5.3. Sub: SYSEX_DIGITAL_PIN_DATA format](#item8)<br>
[5.4. Sub: SYSEX_ANALOG_PIN_DATA format](#item9)<br>
[5.6. Sub: SYSEX_ONEWIRE_DATA format](#item10)<br>
[5.7. Sub: SYSEX_UART_DATA format](#item11)<br>
[5.8. Sub: SYSEX_I2C_DATA format](#item12)<br>
[5.9. Sub: SYSEX_SPI_DATA format](#item13)<br>
[5.10. Sub: SYSEX_STRING_DATA format](#item14)<br>
[5.11. Sub: SYSEX_SCHEDULER_DATA format](#item15)<br>
[5.12. Sub: SYSEX_DIGITAL_PIN_REPORT format](#item16)<br>
[5.13. Sub: SYSEX_ANALOG_PIN_REPORT format](#item17)<br>
[5.14. Sub: SYSEX_VERSION_REPORT format](#item18)<br>
[5.15. Sub: SYSEX_FEATURES_REPORT format](#item19)<br>
[5.16. Sub: SYSEX_PINCAPS_REQ format](#item20)<br>
[5.17. Sub: SYSEX_PINCAPS_REP format](#item21)<br>
[5.18. Sub: SYSEX_PINMAP_REQ format](#item22)<br>
[5.19. Sub: SYSEX_PINMAP_REP format](#item23)<br>
[5.20. Sub: SYSEX_PINSTATE_REQ format](#item24)<br>
[5.21. Sub: SYSEX_PINSTATE_REP format](#item25)<br>
[5.22. Sub: SYSEX_DEVICE_REQ format](#item26)<br>
[5.23. Sub: SYSEX_DEVICE_REP format](#item27)<br>
[5.24. Sub: SYSEX_RCSWITCH_IN format](#item28)<br>
[5.25. Sub: SYSEX_RCSWITCH_OUT format](#item29)<br>

<a name="item1"/>

## 1. Control bytes (0x80-0xFF)
|events with pin/port data     | event byte  |description                |
|------------------------------|-------------|---------------------------|
| STATUS_PIN_MODE              | 0x80 + pin# |set mode (input/output/...)|
| STATUS_DIGITAL_PORT_REPORT   | 0x90 + port#|report port value          |
| STATUS_DIGITAL_PORT_SET      | 0xA0 + port#|set port value             |
| STATUS_DIGITAL_PIN_REPORT    | 0xB0 + pin# |report preferred pin value |
| STATUS_DIGITAL_PIN_SET       | 0xC0 + pin# |set preferred pin value    |
| STATUS_ANALOG_PIN_REPORT     | 0xD0 + pin# |report preferred pin value |
| STATUS_ANALOG_PIN_SET        | 0xE0 + pin# |set preferred pin value    |

|events without pin/port data  | event byte  |description                 |
|------------------------------|-------------|----------------------------|
| STATUS_PROTOCOL_VERSION      | 0xF0        |report the protocol version |
| STATUS_PROTOCOL_ENCODING     | 0xF1        |report the protocol encoding|
| STATUS_INFO                  | 0xF2        |report info (see codes)     |
| STATUS_SIGNAL                | 0xF3        |emit 1-byte 1-way signal    |
| STATUS_INTERRUPT             | 0xF4        |emit 1-byte 1-way interrupt |
| STATUS_CUSTOM_F5             | 0xF5        |user custom event           |
| STATUS_CUSTOM_F6             | 0xF6        |user custom event           |
| STATUS_CUSTOM_F7             | 0xF7        |user custom event           |
| STATUS_CUSTOM_F8             | 0xF8        |user custom event           |
| STATUS_EMERGENCY_STOP        | 0xF9        |emergency stop              |
| STATUS_SYSTEM_PAUSE          | 0xFA        |system pause                |
| STATUS_SYSTEM_RESUME         | 0xFB        |system resume               |
| STATUS_SYSTEM_RESET          | 0xFC        |system reset                |

|special control bytes| event byte  |description          |
|---------------------|-------------|---------------------|
| STATUS_EVENT_BEGIN  | 0xFD        |event begin byte     |
| STATUS_SYSEX_END    | 0xFE        |end of sysex byte    |
| STATUS_SYSEX_START  | 0xFF        |start of sysex byte  |

<a name="item2"/>

## 2. Sysex types (and modifiers)
|type (or modifier)      | value  |
|------------------------|--------|
|SYSEX_PREFERRED_PINS_DATA|0x00   |
|SYSEX_PINGROUPS_DATA     |0x01   |
|SYSEX_DIGITAL_PIN_DATA   |0x02   |
|SYSEX_ANALOG_PIN_DATA    |0x03   |
|SYSEX_ONEWIRE_DATA       |0x04   |
|SYSEX_UART_DATA          |0x05   |
|SYSEX_I2C_DATA           |0x06   |
|SYSEX_SPI_DATA           |0x07   |
|SYSEX_STRING_DATA        |0x08   |
|SYSEX_SCHEDULER_DATA     |0x09   |
|SYSEX_DIGITAL_PIN_REPORT |0x20   |
|SYSEX_ANALOG_PIN_REPORT  |0x21   |
|SYSEX_VERSION_REPORT     |0x22   |
|SYSEX_FEATURES_REPORT    |0x23   |
|SYSEX_PINCAPS_REQ        |0x40   |
|SYSEX_PINCAPS_REP        |0x41   |
|SYSEX_PINMAP_REQ         |0x42   |
|SYSEX_PINMAP_REP         |0x43   |
|SYSEX_PINSTATE_REQ       |0x44   |
|SYSEX_PINSTATE_REP       |0x45   |
|SYSEX_DEVICE_REQ         |0x46   |
|SYSEX_DEVICE_REP         |0x47   |
|SYSEX_RCSWITCH_IN        |0x48   |
|SYSEX_RCSWITCH_OUT       |0x49   |
|(to be defined)          |0x60-0x6F|
|SYSEX_MOD_ASYNC          |0x7D   |
|SYSEX_MOD_SYNC           |0x7E   |
|SYSEX_MOD_EXTEND         |0x7F   |

<a name="item3"/>

## 3. Simple event format
|byte no| value            |
|-------|------------------|
| 0     |STATUS_EVENT_BEGIN|
| 1     |sequenceId        |
| 2     |control byte      |
| 3     |data byte 1       |
| 4     |data byte 2       |
| 5     |CRC8              |

<a name="item4"/>

## 4. Sysex event format
|byte no| value            |
|-------|------------------|
| 0     |STATUS_EVENT_BEGIN|
| 1     |sequenceId        |
| 2     |STATUS_SYSEX_START|
| 3+1   |data byte 1 (mod) |
| 3+2   |data byte 2 (type)|
| 3+3   |data byte 3       |
| 3+N   |data byte N       |
| 3+N+1 |STATUS_SYSEX_END  |
| 3+N+2 |CRC8              |

<a name="item5"/>

## 5. Sysex data format
|byte no| value           |
|-------|-----------------|
| 0     |modifier         |
| 1     |type             |
| 3+1   |data byte 1 (sub)|
| 3+2   |data byte 2      |
| 3+N   |data byte N      |

<a name="item6"/>

## 5.1. Sub: SYSEX_PREFERRED_PINS_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item7"/>

## 5.2. Sub: SYSEX_PINGROUPS_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item8"/>

## 5.3. Sub: SYSEX_DIGITAL_PIN_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item9"/>

## 5.4. Sub: SYSEX_ANALOG_PIN_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item10"/>

## 5.5. Sub: SYSEX_ONEWIRE_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item11"/>

## 5.6. Sub: SYSEX_UART_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item12"/>

## 5.7. Sub: SYSEX_I2C_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item13"/>

## 5.8. Sub: SYSEX_SPI_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item14"/>

## 5.9. Sub: SYSEX_STRING_DATA format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item15"/>

## 5.10. Sub: SYSEX_SCHEDULER_DATA format
|byte no| value                |
|-------|----------------------|
|0      |SYSEX_SUB_SCHED_CREATE|
|1      |task id               |
|2      |task length MSB       |
|3      |task length LSB       |

|byte no| value                |
|-------|----------------------|
|0      |SYSEX_SUB_SCHED_DELETE|
|1      |task id               |

|byte no| value             |
|-------|-------------------|
|0      |SYSEX_SUB_SCHED_ADD|
|1      |task id            |
|2      |task data byte 1   |
|3      |task data byte 2   |
|n      |task data byte n   |

|byte no| value               |
|-------|---------------------|
|0      |SYSEX_SUB_SCHED_DELAY|
|1      |time byte 1          |
|2      |time byte 2          |
|n      |time byte n          |

|byte no| value                  |
|-------|------------------------|
|0      |SYSEX_SUB_SCHED_SCHEDULE|
|1      |task id                 |
|1      |time byte 1             |
|2      |time byte 2             |
|n      |time byte n             |

|byte no| value                  |
|-------|------------------------|
|0      |SYSEX_SUB_SCHED_LIST_REQ|

|byte no| value                  |
|-------|------------------------|
|0      |SYSEX_SUB_SCHED_LIST_REP|
|1      |task id 1               |
|2      |task id 2               |
|n      |task id n               |

|byte no| value                  |
|-------|------------------------|
|0      |SYSEX_SUB_SCHED_TASK_REQ|
|1      |task id                 |

|byte no| value                  |
|-------|------------------------|
|0      |SYSEX_SUB_SCHED_TASK_REP|
|1      |task id                 |
|1      |time byte 1             |
|2      |time byte 2             |
|n      |time byte n             |
|1      |len byte 1              |
|2      |len byte 2              |
|m      |len byte m              |
|1      |pos byte 1              |
|2      |pos byte 2              |
|l      |pos byte l              |
|1      |data byte 1             |
|2      |data byte 2             |
|i      |data byte i             |

|byte no| value                   |
|-------|-------------------------|
|0      |SYSEX_SUB_SCHED_ERROR_REP|
|1      |task id                  |

|byte no| value               |
|-------|---------------------|
|0      |SYSEX_SUB_SCHED_RESET|

<a name="item16"/>

## 5.11. Sub: SYSEX_DIGITAL_PIN_REPORT format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item17"/>

## 5.12. Sub: SYSEX_ANALOG_PIN_REPORT format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item18"/>

## 5.13. Sub: SYSEX_VERSION_REPORT format
|byte no| value                         |
|-------|-------------------------------|
|0      |SYSEX_SUB_VERSION_FIRMWARE_NAME|
|1      |(any amount of chars)          |

|byte no| value                        |
|-------|------------------------------|
|0      |SYSEX_SUB_VERSION_FIRMWARE_VER|
|n      |(git rev-parse --short)       |

|byte no| value                  |
|-------|------------------------|
|0      |SYSEX_SUB_VERSION_LIBKNP|
|n      |(git rev-parse --short) |

|byte no| value                    |
|-------|--------------------------|
|0      |SYSEX_SUB_VERSION_PROTOCOL|
|n      |(git rev-parse --short)   |

|byte no| value                 |
|-------|-----------------------|
|0      |SYSEX_SUB_VERSION_HAL  |
|n      |(git rev-parse --short)|

|byte no| value                 |
|-------|-----------------------|
|0      |SYSEX_SUB_VERSION_BOARD|
|n      |(git rev-parse --short)|

|byte no| value                 |
|-------|-----------------------|
|0      |SYSEX_SUB_VERSION_ARCH |
|n      |(git rev-parse --short)|

|byte no| value                   |
|-------|-------------------------|
|0      |SYSEX_SUB_VERSION_DEFINES|
|n      |(git rev-parse --short)  |

|byte no| value                 |
|-------|-----------------------|
|0      |SYSEX_SUB_VERSION_ALL  |

<a name="item19"/>

## 5.14. Sub: SYSEX_FEATURES_REPORT format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item20"/>

## 5.15. Sub: SYSEX_PINCAPS_REQ format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item21"/>

## 5.16. Sub: SYSEX_PINCAPS_REP format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item22"/>

## 5.17. Sub: SYSEX_PINMAP_REQ format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item23"/>

## 5.18. Sub: SYSEX_PINMAP_REP format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item24"/>

## 5.19. Sub: SYSEX_PINSTATE_REQ format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item25"/>

## 5.20. Sub: SYSEX_PINSTATE_REP format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item26"/>

## 5.21. Sub: SYSEX_DEVICE_REQ format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item27"/>

## 5.22. Sub: SYSEX_DEVICE_REP format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item28"/>

## 5.23. Sub: SYSEX_RCSWITCH_IN format
|byte no| value     |
|-------|-----------|
|0      |WiP        |

<a name="item29"/>

## 5.24. Sub: SYSEX_RCSWITCH_OUT format
|byte no| value     |
|-------|-----------|
|0      |WiP        |
