
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

	// simple: system commands
	void cmdSystemReset(void); // initialize a known default state
	void cmdReportProtocolVersion(void);
	// simple: pin commands
	void cmdPinMode(uint8_t pin, uint8_t mode);
	void cmdPinAnalog(uint8_t pin, uint8_t value);
	void cmdPinDigital(uint8_t port, uint8_t value);
	void cmdPinDigitalSetValue(uint8_t pin, uint8_t value);
	void cmdReportPinAnalog(uint8_t pin, uint8_t en);
	void cmdReportPinDigital(uint8_t port, uint8_t en);

	// sysex: system commands
	void cmdReportFirmwareVersion(void);

	// sysex: fast remote procedure calls
	void cmdRealtimeServer(uint16_t timeout); // starts realtime server
	uint8_t realtimeClient(uint8_t rpc); // issue realtime commands

	// sysex sub: scheduler commands
	void cmdCreateTask(uint8_t id, int len);
	void cmdDeleteTask(uint8_t id);
	void cmdAddToTask(uint8_t id, int additionalBytes, uint8_t *message);
	void cmdScheduleTask(uint8_t id, long delay_ms);
	void cmdDelayTask(uint16_t delay);
	void cmdQueryAllTasks(void);
	void cmdQueryTask(uint8_t id);
	void cmdSchedulerReset(void);

	// run given cmd (or last msg if *msg==NULL)
	void readCmd(uint8_t len, uint8_t *msg);
	// send messages
	// note: for simple messages argc=byte1 and argv=&byte2
	// note: for sysex cmd = CMD_START_SYSEX and argv[0] sysex command
	void writeCmd(uint8_t cmd, uint8_t argc, uint8_t *argv);

	// run scheduled tasks (if any), at the right time
	void runScheduler(void);

	// init&run examples
	void init(void);
	void run(void);

#ifdef __cplusplus
}
#endif

#endif

