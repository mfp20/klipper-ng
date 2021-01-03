// Basic scheduling functions and startup/shutdown code.
//
// Copyright (C) 2016,2017  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <setjmp.h> // setjmp
#include <stdarg.h> // va_start

#include "sched.h" // sched_check_periodic

#include "generic_io.h" // readb
#include "cmd.h" // misc defines
#include "cmds_base.h" // stats_update
#include "rxtx_irq.h" //

extern voidPtr initFunc[FUNCS_INIT_NO];
extern voidPtr taskFunc[FUNCS_TASK_NO];
extern voidPtr endFunc[FUNCS_END_NO];
extern cmdPtr cmdFunc[TYPE_NO];

/****************************************************************
 * Timers
 ****************************************************************/

static struct timer_s periodic_timer, *timer_list = &periodic_timer;
static struct timer_s sentinel_timer, deleted_timer;

// The periodic_timer simplifies the timer code by ensuring there is
// always a timer on the timer list and that there is always a timer
// not far in the future.
static uint_fast8_t periodic_event(struct timer_s *t) {
    // Make sure the stats task runs periodically
    sched_wake_tasks();
    // Reschedule timer
    periodic_timer.waketime += timer_from_us(100000);
    sentinel_timer.waketime = periodic_timer.waketime + 0x80000000;
    return SF_RESCHEDULE;
}

static struct timer_s periodic_timer = {
    .func = periodic_event,
    .next = &sentinel_timer,
};

// The sentinel timer is always the last timer on timer_list - its
// presence allows the code to avoid checking for NULL while
// traversing timer_list.  Since sentinel_timer.waketime is always
// equal to (periodic_timer.waketime + 0x80000000) any added timer
// must always have a waketime less than one of these two timers.
static uint_fast8_t sentinel_event(struct timer_s *t) {
    sched_shutdown(ERROR_SENTINEL_TIMER);
}

static struct timer_s sentinel_timer = {
    .func = sentinel_event,
    .waketime = 0x80000000,
};

// Find position for a timer in timer_list and insert it
static void __always_inline insert_timer(struct timer_s *t, uint32_t waketime) {
    struct timer_s *prev, *pos = timer_list;
    for (;;) {
        prev = pos;
        if (CONFIG_MACH_AVR)
            // micro optimization for AVR - reduces register pressure
            asm("" : "+r"(prev));
        pos = pos->next;
        if (timer_is_before(waketime, pos->waketime))
            break;
    }
    t->next = pos;
    prev->next = t;
}

// Schedule a function call at a supplied time.
void sched_add_timer(struct timer_s *add) {
    uint32_t waketime = add->waketime;
    irqstatus_t flag = irq_save();
    if (unlikely(timer_is_before(waketime, timer_list->waketime))) {
        // This timer is before all other scheduled timers
        struct timer_s *tl = timer_list;
        if (timer_is_before(waketime, timer_read_time()))
            sched_try_shutdown(ERROR_CLOSE_TIMER);
        if (tl == &deleted_timer)
            add->next = deleted_timer.next;
        else
            add->next = tl;
        deleted_timer.waketime = waketime;
        deleted_timer.next = add;
        timer_list = &deleted_timer;
        timer_kick();
    } else {
        insert_timer(add, waketime);
    }
    irq_restore(flag);
}

// The deleted timer is used when deleting an active timer.
static uint_fast8_t deleted_event(struct timer_s *t) {
    return SF_DONE;
}

static struct timer_s deleted_timer = {
    .func = deleted_event,
};

// Remove a timer that may be live.
void sched_del_timer(struct timer_s *del) {
    irqstatus_t flag = irq_save();
    if (timer_list == del) {
        // Deleting the next active timer - replace with deleted_timer
        deleted_timer.waketime = del->waketime;
        deleted_timer.next = del->next;
        timer_list = &deleted_timer;
    } else {
        // Find and remove from timer list (if present)
        struct timer_s *pos;
        for (pos = timer_list; pos->next; pos = pos->next) {
            if (pos->next == del) {
                pos->next = del->next;
                break;
            }
        }
    }
    irq_restore(flag);
}

// Invoke the next timer - called from board hardware irq code.
unsigned int sched_timer_dispatch(void) {
    // Invoke timer callback
    struct timer_s *t = timer_list;
    uint_fast8_t res;
    uint32_t updated_waketime;
    if (CONFIG_INLINE_STEPPER_HACK && likely(!t->func)) {
        // TODO res = stepper_event(t);
		res = 0;
    } else {
        res = t->func(t);
    }
    updated_waketime = t->waketime;

    // Update timer_list (rescheduling current timer if necessary)
    unsigned int next_waketime = updated_waketime;
    if (unlikely(res == SF_DONE)) {
        next_waketime = t->next->waketime;
        timer_list = t->next;
    } else if (!timer_is_before(updated_waketime, t->next->waketime)) {
        next_waketime = t->next->waketime;
        timer_list = t->next;
        insert_timer(t, updated_waketime);
    }

    return next_waketime;
}

// Remove all user timers
void sched_timer_reset(void) {
    timer_list = &deleted_timer;
    deleted_timer.waketime = periodic_timer.waketime;
    deleted_timer.next = &periodic_timer;
    periodic_timer.next = &sentinel_timer;
    timer_kick();
}


/****************************************************************
 * Tasks
 ****************************************************************/

static int_fast8_t tasks_status;

#define TS_IDLE      -1
#define TS_REQUESTED 0
#define TS_RUNNING   1

// Note that at least one task is ready to run
void sched_wake_tasks(void) {
    tasks_status = TS_REQUESTED;
}

// Check if tasks need to be run
uint8_t sched_tasks_busy(void) {
    return tasks_status >= TS_REQUESTED;
}

// Note that a task is ready to run
void sched_wake_task(struct task_wake *w) {
    sched_wake_tasks();
    writeb(&w->wake, 1);
}

// Check if a task is ready to run (as indicated by sched_wake_task)
uint8_t sched_check_wake(struct task_wake *w) {
    if (!readb(&w->wake))
        return 0;
    writeb(&w->wake, 0);
    return 1;
}

// Main task dispatch loop
static void run_tasks(void) {
    uint32_t start = timer_read_time();
    for (;;) {
        // Check if can sleep
        if (tasks_status != TS_REQUESTED) {
            start -= timer_read_time();
            irq_disable();
            if (tasks_status != TS_REQUESTED) {
                // Sleep processor (only run timers) until tasks woken
                tasks_status = TS_IDLE;
                do {
                    irq_wait();
                } while (tasks_status != TS_REQUESTED);
            }
            irq_enable();
            start += timer_read_time();
        }
        tasks_status = TS_RUNNING;

        // Run all tasks
		for (uint8_t i=0;i<FUNCS_TASK_NO;i++) {
			irq_poll();
			taskFunc[i]();
		}

		// Update statistics
		uint32_t cur = timer_read_time();
		stats_update(start, cur);
		start = cur;
	}
}


/****************************************************************
 * Shutdown processing
 ****************************************************************/

static uint_fast8_t shutdown_status, shutdown_reason;

// Return true if the machine is in an emergency stop state
uint8_t sched_is_shutdown(void) {
	return !!shutdown_status;
}

// Transition out of shutdown state
void sched_clear_shutdown(void) {
	if (!shutdown_status)
		sched_shutdown(ERROR_ALREADY_CLEARED);
	if (shutdown_status == 2)
		// Ignore attempt to clear shutdown if still processing shutdown
		return;
	shutdown_status = 0;
}

// Invoke all shutdown functions
static void run_shutdown(int reason) {
	irq_disable();
	uint32_t cur = timer_read_time();
	if (!shutdown_status)
		shutdown_reason = reason;
	shutdown_status = 2;
	sched_timer_reset();

	// run all shutdown functions
	for (uint8_t i=0;i<FUNCS_END_NO;i++) {
		endFunc[i]();
	}

	shutdown_status = 1;
	irq_enable();

	send_command(TYPE_SHUTDOWN_NOW, cur, shutdown_reason);
}

// Report the last shutdown reason code
void sched_report_shutdown(void) {
	send_command(TYPE_SHUTDOWN_LAST, shutdown_reason);
}

// Shutdown the machine if not already in the process of shutting down
void __always_inline sched_try_shutdown(uint_fast8_t reason) {
	if (!shutdown_status)
		sched_shutdown(reason);
}

static jmp_buf shutdown_jmp;

// Force the machine to immediately run the shutdown handlers
void sched_shutdown(uint_fast8_t reason) {
	irq_disable();
	longjmp(shutdown_jmp, reason);
}


/****************************************************************
 * Startup
 ****************************************************************/

// Main loop of program
void sched_main(void) {
	// configure all functions
	funcs_init();

	// run init functions
	for (uint8_t i=0;i<FUNCS_INIT_NO;i++) {
		initFunc[i]();
	}

	send_command(TYPE_START);

	irq_disable();
	int ret = setjmp(shutdown_jmp);
	if (ret)
		run_shutdown(ret);
	irq_enable();

	run_tasks();
}


/****************************************************************
 * encoding/decoding int (compression/decompression)
 ****************************************************************/

// Encode an integer as a variable length quantity (vlq)
uint8_t *vlq_encode(uint8_t *p, uint32_t v) {
	int32_t sv = v;
	if (sv < (3L<<5)  && sv >= -(1L<<5))  goto f4;
	if (sv < (3L<<12) && sv >= -(1L<<12)) goto f3;
	if (sv < (3L<<19) && sv >= -(1L<<19)) goto f2;
	if (sv < (3L<<26) && sv >= -(1L<<26)) goto f1;
	*p++ = (v>>28) | 0x80;
f1: *p++ = ((v>>21) & 0x7f) | 0x80;
f2: *p++ = ((v>>14) & 0x7f) | 0x80;
f3: *p++ = ((v>>7) & 0x7f) | 0x80;
f4: *p++ = v & 0x7f;
	return p;
}

// Parse an integer that was encoded as a "variable length quantity"
uint32_t vlq_decode(uint8_t **pp) {
	uint8_t *p = *pp, c = *p++;
	uint32_t v = c & 0x7f;
	if ((c & 0x60) == 0x60)
		v |= -0x20;
	while (c & 0x80) {
		c = *p++;
		v = (v<<7) | (c & 0x7f);
	}
	*pp = p;
	return v;
}


/****************************************************************
 * send command/response
 ****************************************************************/

// encode and transmit a "command" message
void send_command(uint8_t typ, ...) {
	if (typ >= TYPE_NO); // TODO error

	uint8_t len = 0;
	uint8_t *buf_start = NULL;
	va_list args;
	va_start(args, typ);
	switch (typ) {
		case TYPE_UNKNOWN:
			// TODO
			len = len;
			buf_start = buf_start;
			break;
		case TYPE_NOOP:
			break;
		case TYPE_START:
			break;
		case TYPE_ACK:
			break;
		case TYPE_NACK:
			break;
		case TYPE_SHUTDOWN_NOW:
			break;
		case TYPE_SHUTDOWN_LAST:
			break;
		case TYPE_BASE_IDENTIFY:
			break;
		case TYPE_BASE_ALLOCATE_OIDS:
			break;
		case TYPE_BASE_GET_CONFIG:
			break;
		case TYPE_BASE_FINALIZE_CONFIG:
			break;
		case TYPE_BASE_GET_CLOCK:
			break;
		case TYPE_BASE_GET_UPTIME:
			break;
		case TYPE_BASE_EMERGENCY_STOP:
			break;
		case TYPE_BASE_CLEAR_SHUTDOWN:
			break;
		case TYPE_BASE_STATS:
			break;
		case TYPE_BASE_RESET:
			break;
		case TYPE_DEBUG_START_GROUP:
			break;
		case TYPE_DEBUG_END_GROUP:
			break;
		case TYPE_DEBUG_NOP:
			break;
		case TYPE_DEBUG_PING:
			break;
		case TYPE_DEBUG_READ:
			break;
		case TYPE_DEBUG_WRITE:
			break;
		case TYPE_GPIO_CONFIG:
			break;
		case TYPE_GPIO_SCHEDULE:
			break;
		case TYPE_GPIO_UPDATE:
			break;
		case TYPE_GPIO_SET:
			break;
		case TYPE_GPIO_SOFTPWM_CONFIG:
			break;
		case TYPE_GPIO_SOFTPWM_SCHEDULE:
			break;
		case TYPE_PWM_CONFIG:
			break;
		case TYPE_PWM_SCHEDULE:
			break;
		case TYPE_PWM_SET:
			break;
		case TYPE_ADC_CONFIG:
			break;
		case TYPE_ADC_QUERY:
			break;
		case TYPE_ADC_STATE:
			break;
		case TYPE_I2C_CONFIG:
			break;
		case TYPE_I2C_MODBITS:
			break;
		case TYPE_I2C_READ:
			break;
		case TYPE_I2C_WRITE:
			break;
		case TYPE_SPI_CONFIG:
			break;
		case TYPE_SPI_CONFIG_NOCS:
			break;
		case TYPE_SPI_SET:
			break;
		case TYPE_SPI_TRANSFER:
			break;
		case TYPE_SPI_SEND:
			break;
		case TYPE_SPI_SHUTDOWN:
			break;
		default:
			// TODO error, not implemented
			break;
	}
	va_end(args);
}

// encode and transmit a "response" message
void send_response(uint8_t typ, ...) {
	if (typ >= TYPE_NO); // TODO error

	uint8_t len = 0;
	uint8_t *buf_start = NULL;
	va_list args;
	va_start(args, typ);
	switch (typ) {
		case TYPE_UNKNOWN:
			break;
		case TYPE_NOOP:
			break;
		case TYPE_START:
			break;
		case TYPE_ACK:
		case TYPE_NACK:
			// - An “ack” is a message block with empty content (ie, a 5 byte message block)
			// and a sequence number greater than the last received host sequence number.
			// - A “nak” is a message block with empty content and a sequence number less than the last received host sequence number.
			tx_buffer_trigger(tx_buffer_alloc(len), len);
			break;
		case TYPE_SHUTDOWN_NOW:
			break;
		case TYPE_SHUTDOWN_LAST:
			break;
		case TYPE_BASE_IDENTIFY:
			// TODO
			len = 0;
			buf_start = tx_buffer_alloc(1);
			*(buf_start+MSG_POS_PAYLOAD) = 123;
			len++;
			tx_buffer_trigger(buf_start, len);
			break;
		case TYPE_BASE_ALLOCATE_OIDS:
			break;
		case TYPE_BASE_GET_CONFIG:
			break;
		case TYPE_BASE_FINALIZE_CONFIG:
			break;
		case TYPE_BASE_GET_CLOCK:
			break;
		case TYPE_BASE_GET_UPTIME:
			break;
		case TYPE_BASE_EMERGENCY_STOP:
			break;
		case TYPE_BASE_CLEAR_SHUTDOWN:
			break;
		case TYPE_BASE_STATS:
			break;
		case TYPE_BASE_RESET:
			break;
		case TYPE_DEBUG_START_GROUP:
			break;
		case TYPE_DEBUG_END_GROUP:
			break;
		case TYPE_DEBUG_NOP:
			break;
		case TYPE_DEBUG_PING:
			break;
		case TYPE_DEBUG_READ:
			break;
		case TYPE_DEBUG_WRITE:
			break;
		case TYPE_GPIO_CONFIG:
			break;
		case TYPE_GPIO_SCHEDULE:
			break;
		case TYPE_GPIO_UPDATE:
			break;
		case TYPE_GPIO_SET:
			break;
		case TYPE_GPIO_SOFTPWM_CONFIG:
			break;
		case TYPE_GPIO_SOFTPWM_SCHEDULE:
			break;
		case TYPE_PWM_CONFIG:
			break;
		case TYPE_PWM_SCHEDULE:
			break;
		case TYPE_PWM_SET:
			break;
		case TYPE_ADC_CONFIG:
			break;
		case TYPE_ADC_QUERY:
			break;
		case TYPE_ADC_STATE:
			break;
		case TYPE_I2C_CONFIG:
			break;
		case TYPE_I2C_MODBITS:
			break;
		case TYPE_I2C_READ:
			break;
		case TYPE_I2C_WRITE:
			break;
		case TYPE_SPI_CONFIG:
			break;
		case TYPE_SPI_CONFIG_NOCS:
			break;
		case TYPE_SPI_SET:
			break;
		case TYPE_SPI_TRANSFER:
			break;
		case TYPE_SPI_SEND:
			break;
		case TYPE_SPI_SHUTDOWN:
			break;
		default:
			// TODO error, not implemented
			break;
	}
	va_end(args);
}

