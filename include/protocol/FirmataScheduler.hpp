
#ifndef PROTOCOL_SCHEDULER_H
#define PROTOCOL_SCHEDULER_H

#include <protocol/Firmata.hpp>
#include <protocol/FirmataFeature.hpp>
#include <protocol/Firmata7BitEnc.hpp>

//subcommands
#define CREATE_FIRMATA_TASK     0
#define DELETE_FIRMATA_TASK     1
#define ADD_TO_FIRMATA_TASK     2
#define DELAY_FIRMATA_TASK      3
#define SCHEDULE_FIRMATA_TASK   4
#define QUERY_ALL_FIRMATA_TASKS 5
#define QUERY_FIRMATA_TASK      6
#define RESET_FIRMATA_TASKS     7
#define ERROR_TASK_REPLY        8
#define QUERY_ALL_TASKS_REPLY   9
#define QUERY_TASK_REPLY        10

#define firmata_task_len(a)(sizeof(firmata_task)+(a)->len)

void delayTaskCallback(long delay);

struct firmata_task {
  firmata_task *nextTask;
  uint8_t id; //only 7bits used -> supports 127 tasks
  long time_ms;
  int len;
  int pos;
  uint8_t messages[1];
};

class FirmataScheduler: public FirmataFeature {
  public:
    FirmataScheduler();
    void handleCapability(uint8_t pin); //empty method
    bool handlePinMode(uint8_t pin, int mode); //empty method
    bool handleSysex(uint8_t command, uint8_t argc, uint8_t* argv);
    void runTasks();
    void reset();
    void createTask(uint8_t id, int len);
    void deleteTask(uint8_t id);
    void addToTask(uint8_t id, int len, uint8_t *message);
    void schedule(uint8_t id, long time_ms);
    void delayTask(long time_ms);
    void queryAllTasks();
    void queryTask(uint8_t id);

  private:
    firmata_task *tasks;
    firmata_task *running;

    bool execute(firmata_task *task);
    firmata_task *findTask(uint8_t id);
    void reportTask(uint8_t id, firmata_task *task, bool error);
};

extern Firmata firmata;
extern FirmataScheduler fSched;
extern Firmata7BitEnc fEnc;

#endif

