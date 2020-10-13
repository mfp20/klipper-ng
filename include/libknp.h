
#ifndef LIBKNP_H
#define LIBKNP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hal.h>
#include <protocol/firmata.h>

#define TICKRATE 1000
#define firmata_task_len(a)(sizeof(task_t)+(a)->len)

	typedef struct task_s {
		uint8_t id; // only 7bits used -> supports 127 tasks
		long ms; // ms to task
		int len;
		int pos;
		uint8_t *messages;
		struct task_s *next;
	} task_t;

	extern task_t *tasks;
	extern task_t *running;

	// system commands
	void cmdReportProtocolVersion(void);
	void cmdReportFirmwareVersion(void);
	void cmdSystemReset(void);

	// pin commands
	// TODO

	// scheduler commands
	void cmdCreateTask(uint8_t id, int len);
	void cmdDeleteTask(uint8_t id);
	void cmdAddToTask(uint8_t id, int additionalBytes, uint8_t *message);
	void cmdScheduleTask(uint8_t id, long delay_ms);
	void cmdDelayTask(uint16_t delay);
	void cmdQueryAllTasks(void);
	void cmdQueryTask(uint8_t id);
	void cmdSchedulerReset(void);

	// fast 1-byte remote procedure calls
	void cmdRealtimeServer(uint16_t timeout);
	uint8_t realtimeClient(uint8_t rpc);

	// callbacks to dispatch received commands
	void dispatchSimple(uint8_t cmd, uint8_t byte1, uint8_t byte2);
	void dispatchSysex(uint8_t cmd, uint8_t argc, uint8_t *argv);

	// transmit 1 signal (ie: SysEx) to host
	void signal(uint8_t sig, uint8_t count, uint8_t *data);

	// run scheduled tasks (if any), at the right time
	void scheduler_run(void);

	// init&run examples
	void init(void);
	void run(void);

#ifdef __cplusplus
}
#endif

#endif

