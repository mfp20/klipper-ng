#
# Author: Anichang
# Date  : 2020-01-01
#
# make all
# make clean
# make {all|hal|proto|lib|firmware|host}
# make build
# make release
# make debug
# make help
# 

BOARD ?= simulinux
ifeq ($(BOARD),hostlinux)
	ARCH=linux
	MCU=x86
	CROSS_COMPILE=
endif
ifeq ($(BOARD),simulinux)
	ARCH=bogus
	MCU=bogus
	CROSS_COMPILE=
endif
ifeq ($(BOARD),linux)
	ARCH=linux
	MCU=x86
	CROSS_COMPILE=
endif
ifeq ($(BOARD),rpi)
	ARCH=linux
	MCU=arm
	CROSS_COMPILE=arm-
endif
ifeq ($(BOARD),odroid)
	ARCH=linux
	MCU=arm
	CROSS_COMPILE=arm-
endif
ifeq ($(BOARD),generic328p)
	ARCH=avr
	MCU=atmega328p
	CROSS_COMPILE=avr-
endif
ifeq ($(BOARD),melzi)
	ARCH=avr
	MCU=atmega1284p
	CROSS_COMPILE=avr-
endif
ifeq ($(BOARD),mksgenl)
	ARCH=avr
	MCU=atmega2560
	CROSS_COMPILE=avr-
endif
ARCH ?= linux
MCU ?= x86
CROSS_COMPILE ?=

# tools
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AS = $(CROSS_COMPILE)as
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
STRIP = $(CROSS_COMPILE)strip

# files
DIR_BUILD = ./build
DIR_OBJ	:= $(DIR_BUILD)/objs
DIR_PY := $(DIR_BUILD)/py
DIR_APP	:= $(DIR_BUILD)/bin
#INCLUDE	:= -Iinclude/ -I/usr/avr/include
INCLUDE	:= -Iinclude/

SRCS_UTIL	:= $(wildcard src/utility/*.c)
SRCS_HAL	:= $(wildcard src/utility/*.$(ARCH).c) \
			src/hal/arch.c \
			src/hal/arch.$(ARCH).c \
			src/hal/board.c \
			src/hal/board.$(BOARD).c \
			src/hal.c
SRCS_PROTO	:= src/protocol.c
SRCS_PRINTER	:=
SRCS_LIB	:= src/libknp.c
SRCS_READER	:= src/reader.c
SRCS_WRITER	:= src/writer.c
SRCS_FW		:= src/firmware.$(BOARD).c
SRCS_HOST	:= $(wildcard src/host/chelper/*.c)

OBJS_UTIL	:= $(SRCS_UTIL:%.c=$(DIR_OBJ)/%.$(BOARD).o)
OBJS_HAL	:= $(SRCS_HAL:%.c=$(DIR_OBJ)/%.$(BOARD).o)
OBJS_PROTO	:= $(SRCS_PROTO:%.c=$(DIR_OBJ)/%.$(BOARD).o)
OBJS_PRINTER	:= $(SRCS_PRINTER:%.c=$(DIR_OBJ)/%.$(BOARD).o)
OBJS_LIB	:= $(SRCS_LIB:%.c=$(DIR_OBJ)/%.$(BOARD).o)
OBJS_READER	:= $(SRCS_READER:%.c=$(DIR_OBJ)/%.$(BOARD).o)
OBJS_WRITER	:= $(SRCS_WRITER:%.c=$(DIR_OBJ)/%.$(BOARD).o)
OBJS_FW		:= $(SRCS_FW:%.c=$(DIR_OBJ)/%.$(BOARD).o)
OBJS_HOST	:= $(SRCS_HOST:%.c=$(DIR_OBJ)/%.$(BOARD).o)

# flags
# -fPIC, python needs, avr doesn't support
# -mmcu=$(MCU) avr need it
CFLAGS  = \
	  -fPIC \
	  -D__GIT_REVPARSE__=0x$(shell git rev-parse --short HEAD) \
	  -D__FIRMWARE_ARCH_$(shell echo $(ARCH) | tr a-z A-Z)__ \
	  -D__FIRMWARE_MCU_$(shell echo $(MCU) | tr a-z A-Z)__ \
	  -D__FIRMWARE_BOARD_$(shell echo $(BOARD) | tr a-z A-Z)__ \
	  $(INCLUDE)
LDFLAGS = -pthread -lutil -lm -lrt

# targets
TARGET_HAL	:= $(DIR_OBJ)/libhal.$(BOARD).a
TARGET_PROTO	:= $(DIR_OBJ)/libproto.$(BOARD).a
TARGET_LIB	:= $(DIR_OBJ)/libknp.$(BOARD).a
TARGET_READER	:= $(DIR_APP)/reader.$(BOARD).bin
TARGET_WRITER	:= $(DIR_APP)/writer.$(BOARD).bin
TARGET_FW	:= $(DIR_APP)/klipper-ng.$(BOARD).bin
TARGET_HOST	:= $(DIR_APP)/klippy-ng.elf

help:
	@echo "make all"
	@echo "make {hal|proto|lib|fw|host}"
	@echo "make build"
	@echo "make release"
	@echo "make debug"
	@echo "make clean"
	@echo "make help"
	@echo ""
	@echo "It builds Linux simulator by default."
	@echo "Use BOARD={linux|generic328p|melzi|mksgenl} to build for other boards, example:"
	@echo "		make BOARD=melzi fw"
	@echo "to build the firmware for Melzi board."
	@echo "WARNING: currently Linux simulator is the only supported board."

$(DIR_OBJ)/src/%.$(BOARD).o: src/%.c
	@mkdir -p $(@D)
	cpp $(CFLAGS) -E -dM $< -o $@.i
	$(CC) $(CFLAGS) -c $< -o $@ 

$(TARGET_HAL): $(OBJS_HAL)
	@mkdir -p $(@D)
	cpp $(CFLAGS) -E include/hal/platform.h | grep "^[^#]" > $(DIR_OBJ)/src/hal/platform.h.i
	$(AR) rcs $(TARGET_HAL) $(OBJS_HAL)

$(TARGET_PROTO): $(OBJS_PROTO) 
	@mkdir -p $(@D)
	$(AR) rcs $(TARGET_PROTO) $(OBJS_PROTO)

$(TARGET_LIB): $(TARGET_HAL) $(TARGET_PROTO) $(OBJS_LIB)
	@mkdir -p $(@D)
	$(AR) rcs $(TARGET_LIB) $(OBJS_HAL) $(OBJS_PROTO) $(OBJS_LIB)

$(TARGET_READER): $(TARGET_LIB) $(OBJS_READER)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET_READER) $(OBJS_READER) -L$(DIR_OBJ) -l:libknp.$(BOARD).a

$(TARGET_WRITER): $(TARGET_LIB) $(OBJS_WRITER)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET_WRITER) $(OBJS_WRITER) -L$(DIR_OBJ) -l:libknp.$(BOARD).a

$(TARGET_FW): $(TARGET_LIB) $(OBJS_FW)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET_FW) $(OBJS_FW) -L$(DIR_OBJ) -l:libknp.$(BOARD).a

$(TARGET_HOST): py $(OBJS_HOST)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(DIR_APP)/$(TARGET_HOST) $^ 

.PHONY: all build clean debug release

build:
	@mkdir -p $(DIR_APP)
	@mkdir -p $(DIR_OBJ)
	@mkdir -p $(DIR_PY)

compiledb:
	@$(shell compiledb -n make lib)

all: build hal proto lib fw py host

hal: build $(TARGET_HAL)

proto: build $(TARGET_PROTO)

lib: build $(TARGET_LIB)

fw: build $(TARGET_FW)

reader: build
	@make BOARD=hostlinux $(DIR_APP)/reader.hostlinux.bin

writer: build
	@make BOARD=hostlinux $(DIR_APP)/writer.hostlinux.bin

py:
	@make BOARD=hostlinux $(DIR_OBJ)/libknp.hostlinux.a
	@python Makefile.py

host: build
	@make BOARD=hostlinux $(DIR_OBJ)/libknp.hostlinux.a

generic328p:
	@make BOARD=generic328p all
generic328p_fw:
	@make BOARD=generic328p fw
generic328p_py:
	@make BOARD=generic328p py

debug: CFLAGS += -pedantic-errors -Wall -Werror -Wextra -DDEBUG -g
debug: all

release: CFLAGS += -O2
release: all

clean:
	-@rm -rvf $(DIR_OBJ)/*
	-@rm -rvf $(DIR_PY)/*
	-@rm -rvf $(DIR_APP)/*

