#
# Author: Anichang
# Date  : 2020-01-01

ifndef BOARD
	BOARD=linux
endif
ifeq ($(BOARD),linux)
	MCU=x86
	CROSS_COMPILE=''
endif
ifeq ($(BOARD),generic328p)
	MCU=atmega328p
	CROSS_COMPILE=avr-
endif
ifeq ($(BOARD),melzi)
	MCU=atmega1284p
	CROSS_COMPILE=avr-
endif
ifeq ($(BOARD),mksgenl)
	MCU=atmega2560
	CROSS_COMPILE=avr-
endif
ifndef MCU
	MCU=x86
endif
ifndef CROSS_COMPILE
	CROSS_COMPILE=''
endif

# tools
CC = $(CROSS_PREFIX)gcc
CPP = $(CROSS_PREFIX)g++
AS = $(CROSS_PREFIX)as
LD = $(CROSS_PREFIX)ld
OBJCOPY = $(CROSS_PREFIX)objcopy
OBJDUMP = $(CROSS_PREFIX)objdump
STRIP = $(CROSS_PREFIX)strip

# flags
CFLAGS  = -std=gnu++17 -O2 -g -Wall
CPPFLAGS  = $(CFLAGS)
LDFLAGS = $(CPPFLAGS) -lm -pthread -lrt -lutil

# sources
SRCS_HAL = hal/arch.c hal/board.c
SRCS_PROTOCOL = protocol/ConfigurableFirmata.cpp
SRCS = $(SRCS_HAL) $(SRCS_PROTOCOL)

# intermediate binaries
OBJS_HAL = $(SRCS_HAL:hal/%.c=%.c.o) 
OBJS_PROTOCOL = $(SRCS_PROTOCOL:protocol/%.cpp=%.cpp.o) 
OBJS = $(OBJS_HAL) $(OBJS_PROTOCOL) 

# target
all: $(PLATFORMS)

%.c.o: %.c
	@$(CC) $(CCFLAGS) -c $< -o $@
	@echo $<" OK."

%.cpp.o: %.cpp
	@$(CPP) $(CPPFLAGS) -c $< -o $(OUT)$@
	@echo $<" OK."

libknp:
	@echo "BOARD: $(BOARD)"
	@echo "MCU: $(MCU)"
	@echo "OBJS: $(OBJS_PROTOCOL)"
	@$(CPP) $(CPPFLAGS) -c protocol/ConfigurableFirmata.cpp -o $(OUT)ConfigurableFirmata.o
#	$(CPP) $(LDFLAGS) -o libknp.$(BOARD).a $(OBJS_PROTOCOL) 

linux: libknp #$(OBJS_HAL)
	@echo "BOARD: $(BOARD)"
	@echo "MCU: $(MCU)"
#	$(CC) $(LDFLAGS) -o klipper-next.$(BOARD).elf $(OBJS_HAL) 

generic328p: libknp $(OBJS_HAL)
	@echo "BOARD: $(BOARD)"
	@echo "MCU: $(MCU)"
#	$(CC) $(LDFLAGS) -o klipper-next.$(BOARD).bin $(OBJS_HAL) 

melzi: libknp $(OBJS_HAL)
	@echo "BOARD: $(BOARD)"
	@echo "MCU: $(MCU)"
#	$(CC) $(LDFLAGS) -o klipper-next.$(BOARD).bin $(OBJS_HAL) 

mksgenl: libknp $(OBJS_HAL)
	@echo "BOARD: $(BOARD)"
	@echo "MCU: $(MCU)"
#	$(CC) $(LDFLAGS) -o klipper-next.$(BOARD).bin $(OBJS_HAL) 

clean: 
	$(RM) $(OBJS)

