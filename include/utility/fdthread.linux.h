#ifndef HAL_H
#define HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <sys/epoll.h>

#define FD_TYPE_FILE	0
#define FD_TYPE_PTY		1
#define FD_TYPE_TTY		2
#define FD_TYPE_SOCKET	3

#define BLOCK_SIZE 64

typedef struct fd_s {
	int type;
	struct epoll_event event;
	int pos;
	uint8_t *rx_buffer;
	int rx_buffer_len;
	uint8_t *tx_buffer;
	int tx_buffer_len;
} fd_t;

typedef void (*fptr_alarm_t)(int fdid);

extern const char *fd_pty_filename;
extern int fd_idx;
extern fptr_alarm_t rBufferFullHandler;
extern fptr_alarm_t wBufferFullHandler;

int initFdThread(void);
int fdCreateFile(const char *fname);
int fdCreatePty(const char *fname);
int fdCreateSocket(const char *fname);
int fdOpen(const char *fname, uint8_t type);
int fdGet(int fdid);
int fdAvailable(int fdid);
int fdRead(int fdid, uint8_t *data);
int fdWrite(int fdid, uint8_t *data, int len);
int fdClose(int fdid);
int closeFdThread(void);

#ifdef __cplusplus
}
#endif

#endif

