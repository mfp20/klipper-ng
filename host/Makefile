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
CFLAGS  = -g -Wall
CPPFLAGS  = $(CFLAGS)
LDFLAGS = $(CPPFLAGS) -lm -pthread -lrt -lutil

SRCS = arch.c board.c main.c
OBJS = $(SRCS:%.c=%.o)

all: $(PLATFORMS)

libknp:
	@make $(COMMON_VARS) BOARD=linux -C ../firmware libknp

linux:
	@echo "BOARD: $(BOARD)"
	@echo "MCU: $(MCU)"
#	$(CPP) $(LDFLAGS) -o klipper-next.elf $(OBJS) 

%.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo $<" OK."

clean: 
	$(RM) $(OBJS)

