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
ifeq ($(BOARD),simulinux)
	MCU=bogus
	CROSS_COMPILE=
endif
ifeq ($(BOARD),genericx86)
	MCU=x86
	CROSS_COMPILE=
endif
ifeq ($(BOARD),rpi)
	MCU=arm
	CROSS_COMPILE=arm-
endif
ifeq ($(BOARD),odroid)
	MCU=arm
	CROSS_COMPILE=arm-
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
DIR_APP	:= $(DIR_BUILD)/apps
INCLUDE	:= -Iinclude/
SRCS_UTIL	:= \
	$(wildcard src/utility/*.c)	\
	$(wildcard src/utility/*.cpp)
SRCS_HAL	:= \
	src/hal/arch_$(MCU).c src/hal/board_$(BOARD).c
SRCS_PROTO	:= \
	src/protocol.c
SRCS_LIB	:= \
	src/libknp.c
SRCS_FW		:= \
	src/firmware_$(BOARD).c
SRCS_HOST	:= \
	$(wildcard src/host/chelper/*.c)
OBJS_UTIL	:= $(SRCS_UTIL:%.c=$(DIR_OBJ)/%.o)
OBJS_HAL	:= $(DIR_OBJ)/src/hal/arch.$(BOARD).o $(DIR_OBJ)/src/hal/board.$(BOARD).o $(DIR_OBJ)/src/hal.$(BOARD).o
OBJS_PROTO	:= $(SRCS_PROTO:%.c=$(DIR_OBJ)/%.o)
OBJS_LIB	:= $(DIR_OBJ)/src/libknp.$(BOARD).o
OBJS_FW		:= $(SRCS_FW:%.cpp=$(DIR_OBJ)/%.o)
OBJS_HOST	:= $(SRCS_HOST:%.c=$(DIR_OBJ)/%.o) $(SRCS_HOST:%.cpp=$(DIR_OBJ)/%.o)

# flags
CFLAGS  = $(INCLUDE) -D__FIRMWARE_ARCH_$(shell echo $(MCU) | tr a-z A-Z)__ -D__FIRMWARE_BOARD_$(shell echo $(BOARD) | tr a-z A-Z)__
CXXFLAGS = $(CFLAGS) -std=gnu++17
LDFLAGS = -L$(DIR_BUILD) -L$(DIR_OBJ) -L$(DIR_APP) -lstdc++ -pthread -lutil -lm -lrt

# targets
TARGET_HAL	:= $(DIR_OBJ)/libhal.$(BOARD).a
TARGET_PROTO	:= $(DIR_OBJ)/libproto.$(BOARD).a
TARGET_LIB	:= $(DIR_APP)/libknp.$(BOARD).a
TARGET_FW	:= $(DIR_APP)/klipper-ng.$(BOARD).bin
TARGET_HOST	:= $(DIR_APP)/klipper-ng.elf

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

all: build hal proto lib fw host
hal: build $(TARGET_HAL)
proto: build $(TARGET_PROTO)
lib: build $(TARGET_LIB)
fw: build $(TARGET_FW)
host: build $(TARGET_HOST)

$(DIR_OBJ)/src/hal/arch.$(BOARD).o: src/hal/arch_$(MCU).c
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(DIR_OBJ)/src/hal/board.$(BOARD).o: src/hal/board_$(BOARD).c
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(DIR_OBJ)/src/hal.$(BOARD).o: src/hal.c
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(DIR_OBJ)/%.o: %.c
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $(@:%.o=%.$(BOARD).o) $(LDFLAGS)

$(DIR_OBJ)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $(@:%.o=%.$(BOARD).o) $(LDFLAGS)

$(TARGET_HAL): $(OBJS_UTIL) $(OBJS_HAL)
	@mkdir -p $(@D)
	$(AR) rcs $(TARGET_HAL) $(OBJS_UTIL:%.o=%.$(BOARD).o) $(OBJS_HAL)

$(TARGET_PROTO): $(OBJS_UTIL) $(OBJS_PROTO) 
	@mkdir -p $(@D)
	$(AR) rcs $(TARGET_PROTO) $(OBJS_UTIL:%.o=%.$(BOARD).o) $(OBJS_PROTO:%.o=%.$(BOARD).o)

$(TARGET_LIB): $(TARGET_HAL) $(TARGET_PROTO)
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c src/libknp.c -o $(DIR_OBJ)/src/libknp.$(BOARD).o $(LDFLAGS)
	$(AR) rcs $(TARGET_LIB) $(OBJS_UTIL:%.o=%.$(BOARD).o) $(OBJS_HAL) $(OBJS_PROTO:%.o=%.$(BOARD).o) $(OBJS_LIB)

$(TARGET_FW): $(TARGET_LIB)
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -o $(TARGET_FW) $(SRCS_FW) $(LDFLAGS) -l:libknp.$(BOARD).a

$(TARGET_HOST): $(OBJS_HOST)
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -o $(DIR_APP)/$(TARGET_HOST) $^ $(LDFLAGS)

.PHONY: all build clean debug release

build:
	@mkdir -p $(DIR_APP)
	@mkdir -p $(DIR_OBJ)

compiledb:
	@$(shell compiledb -n make fw)

debug: CXXFLAGS += -pedantic-errors -Wall -Werror -Wextra -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(DIR_OBJ)/*
	-@rm -rvf $(DIR_APP)/*

