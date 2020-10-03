
#ifndef SCHED_H
#define SCHED_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "platform/board.h"

// memo: USB 2.0 minimum polling interval is 125us

typedef struct action_s {
	void (*func)(uint8_t argc, char *argv[]);
	uint8_t argc;
	char *argv;
} action_t;

typedef struct event_s {
	uint16_t evid;
	uint16_t second;
	uint16_t milli;
	action_t *action;
	struct event_s *next;
} event_t;

typedef struct event_queue_s {
	event_t *head;
	event_t *tail;
} event_queue_t;
event_queue_t tasks;

// actions
void noop(uint8_t argc, char *argv[]);
void pong(uint8_t argc, char *argv[]);
void time_reset(uint8_t argc, char *argv[]);
void time_sync(uint8_t argc, char *argv[]);
void pin_setup(uint8_t argc, char *argv[]);
void pin_get(uint8_t argc, char *argv[]);
void pin_set(uint8_t argc, char *argv[]);
void pulse_get(uint8_t argc, char *argv[]);
void pulse_set(uint8_t argc, char *argv[]);
void report_platform(uint8_t argc, char *argv[]);
void report_nanostamp(uint8_t argc, char *argv[]);
void report_timestamp(uint8_t argc, char *argv[]);
void report_tasks(uint8_t argc, char *argv[]);
void report_pulses(uint8_t argc, char *argv[]);

//
volatile uint8_t lag;
void init(void);
// add 1 event to event queue
void event_add(uint16_t second, uint16_t milli, void *func, uint8_t argc, char *argv[]);
// del 1 event to event queue
void event_del(uint16_t evid);
// reset event queue
void event_reset(void);
void run(void);

#endif

