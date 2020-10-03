
#include "sched.h"

// ACTIONS -------------------------------------------------------------------------

void noop(uint8_t argc, char *argv[]) {}
void pong(uint8_t argc, char *argv[]) {}
void time_sync(uint8_t argc, char *argv[]) {}
void time_reset(uint8_t argc, char *argv[]) {}
void pin_setup(uint8_t argc, char *argv[]) {}
void pin_get(uint8_t argc, char *argv[]) {}
void pin_set(uint8_t argc, char *argv[]) {}
void pulse_get(uint8_t argc, char *argv[]) {}
void pulse_set(uint8_t argc, char *argv[]) {}
void report_platform(uint8_t argc, char *argv[]) {}
void report_nanostamp(uint8_t argc, char *argv[]) {}
void report_timestamp(uint8_t argc, char *argv[]) {}
void report_tasks(uint8_t argc, char *argv[]) {}
void report_pulses(uint8_t argc, char *argv[]) {}


// RUN

static uint16_t event_idx = 0;
void init(void) {
	printf("init()\n");
	board_init();
	tasks.head = NULL;
	tasks.tail = NULL;
}

void event_add(uint16_t second, uint16_t milli, void *func, uint8_t argc, char *argv[]) {
	if (tasks.head == NULL) {
		// add new event to the empty list
		tasks.head = malloc(sizeof(event_t));
		tasks.head->evid = event_idx;
		event_idx++;
		tasks.head->second = second;
		tasks.head->milli = milli;
		tasks.head->action = malloc(sizeof(action_t));
		tasks.head->action->func = func;
		tasks.head->action->argc = argc;
		tasks.head->action->argv = *argv;
		tasks.head->next = NULL;
		tasks.tail = tasks.head;
	} else {
		// search for the right spot in time
		event_t *minor = tasks.tail;
		while(minor != NULL) {
			if ((second < minor->second)&&(milli < minor->milli)) {
				break;
			}
			minor = minor->next;
		}
		if (minor == NULL) minor = tasks.head;
		// add the new event at the right spot
		minor->next = malloc(sizeof(event_t));
		minor->next->evid = event_idx;
		event_idx++;
		minor->next->second = second;
		minor->next->milli = milli;
		minor->next->action = malloc(sizeof(action_t));
		minor->next->action->func = func;
		minor->next->action->argc = argc;
		minor->next->action->argv = *argv;
		tasks.head = minor->next;
		tasks.head->next = NULL;
	}
}

void event_del(uint16_t evid) {
	event_t *parent = NULL;
	event_t *target = tasks.tail;
	while(target != NULL) {
		if (evid == target->evid) {
			break;
		}
		parent = target;
		target = target->next;
	}
	if (target != NULL) {
		parent->next = target->next;
		free(target->action);
		free(target);
	}
}

void event_reset(void) {
	event_t *index = tasks.tail;
	while (index != NULL) {
		index = index->next;
		free(index->action);
		free(index);
	}
	tasks.head = tasks.tail = NULL;
}

static uint16_t tdiff(uint16_t start, uint16_t end) {
	if (start > end) {
		//printf("OVF: %d %d -> %d\n", start, end, (999-start)+end);
		return (999-start)+end;
	} else {
		//printf("OK: %d %d -> %d\n", start, end, end-start);
		return end-start;
	}
}

static uint16_t snow, mnow, unow, udiff;
static uint16_t counter = 0;
void run(void) {
	while(1) {
		// time keeping pre
		unow = micros();
		mnow = millis();
		snow = seconds();
		// system tasks
		uint16_t elapsed = board_run(snow, mnow, 950);
		if (elapsed < 950) {
			// user tasks (if any) ...
			if (tasks.tail != NULL) {
				// ... at the right time
				if ((tasks.tail->second > snow)||(tasks.tail->second == snow && tasks.tail->milli >= mnow)) {
					tasks.tail->action->func(tasks.tail->action->argc,&tasks.tail->action->argv);
					tasks.tail = tasks.tail->next;
				}
			}
			// time keeping post
			counter++;
			udiff = tdiff(unow,micros());
			// report cpu usage
			lag=udiff/10;
			// delay up to 1ms
			if (udiff < 950) usleep(950-udiff);
			printf("%ds:%dms --- TICK ", snow, mnow);
		} else {
			// report cpu usage 100%
			lag = 100;
			// TODO: report delay to host
			printf("%ds:%dms --- LAG ", seconds(), mnow);
		}
		printf("(CPU %d%%)\n", lag);
	}
}

