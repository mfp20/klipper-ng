
# compiler setup
CC = $(CROSS_PREFIX)gcc
AS = $(CROSS_PREFIX)as
LD = $(CROSS_PREFIX)ld
OBJCOPY = $(CROSS_PREFIX)objcopy
OBJDUMP = $(CROSS_PREFIX)objdump
STRIP = $(CROSS_PREFIX)strip
CPP = g++
CFLAGS  = -g -Wall

#
OBJS = cbuffer.o arch.o board.o sched.o
OUT = out/

all: x86 cbuffer.o sched.o
	$(CC) $(CFLAGS) -o klipper-next.o main.c $(OBJS) -lm -pthread -lrt -lutil

x86:
	$(CC) $(CFLAGS) -o arch.o -c platform/arch_x86.c
	$(CC) $(CFLAGS) -o board.o -c platform/board_linux.c

avr:
	$(CC) $(CFLAGS) -o arch.o -c platform/arch_avr.c
	$(CC) $(CFLAGS) -o board.o -c platform/board_avr.c

cbuffer.o:
	$(CC) $(CFLAGS) -o cbuffer.o -c cbuffer.c

sched.o:
	$(CC) $(CFLAGS) -o sched.o -c sched.c

clean: 
	$(RM) *.o *~

