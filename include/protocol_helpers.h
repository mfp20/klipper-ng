
typedef struct task_s {
	uint8_t id; // only 7bits used -> supports 127 tasks
	long ms; // ms to task
	int len;
	int pos;
	uint8_t *messages;
	struct task_s *next;
} task_t;

// callback function types
typedef void (*cbf_varg_t)(const char *format, ...);
typedef void (*cbf_data_t)(uint8_t argc, uint8_t *argv);
typedef uint8_t (*cbf_eval_t)(uint8_t count);
typedef void (*cbf_coder_t)(uint8_t *input, uint8_t count, uint8_t *output);

// callback functions
extern cbf_varg_t printString;
extern cbf_varg_t printErr;
extern cbf_eval_t cbEvalEnc;
extern cbf_coder_t cbEncoder;
extern cbf_eval_t cbEvalDec;
extern cbf_coder_t cbDecoder;
extern cbf_data_t runEvent;

// events
extern uint16_t deltaTime; // ticks since last complete event
// tasks
extern task_t *tasks;
extern task_t *running;

// reset receive buffer
void bufferReset(void);

// copy buffer to event, returns the event size
uint8_t bufferCopy(uint8_t *event);

//
uint8_t encodingSwitch(uint8_t proto);

// receive 1 byte in default buffer (or given buffer)
// - uncomplete event: returns 0,
// - complete event: returns the number of stored bytes
uint8_t decodeEvent(uint8_t *byte, uint8_t size, uint8_t *event);

// prepare data for sending and write it at &event:
//		- for simple events argc=byte1 and argv=&byteN
//		- for sysex events = CMD_START_SYSEX, argv[0] is sysex modifier, and argv[1] sysex event, then data
// returns the event's number of bytes (to be eventually sent)
uint8_t encodeEvent(uint8_t cmd, uint8_t argc, uint8_t *argv, uint8_t *event);

// print event in buffer (or given event)
void printEvent(uint8_t count, uint8_t *event);

// encode subs (task, ...)
uint8_t encodeTask(task_t *task, uint8_t error, uint8_t *event);

// run scheduled tasks (if any), at the right time
void runScheduler(uint16_t now);

