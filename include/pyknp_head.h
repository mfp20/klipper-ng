
double arch_monotonic(void);

typedef void (*fptr_alarm_t)(int fdid);

extern fptr_alarm_t bufferFullHandler;

int initFdThread(void);
int fdOpen(const char *fname, uint8_t type);
int fdGet(int fdid);
int fdAvailable(int fdid);
int fdRead(int fdid, uint8_t *data);
int fdWrite(int fdid, uint8_t *data, int len);
int fdClose(int fdid);
int closeFdThread(void);

