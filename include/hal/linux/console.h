#ifndef __LINUX_CONSOLE_H
#define __LINUX_CONSOLE_H

#include <time.h> // struct timespec

void report_errno(char *where, int rc);
int set_non_blocking(int fd);
int console_setup(char *name);
void console_sleep(struct timespec ts);

void tx_buffer_enable_irq(void);

void task_console_rx(void);

#endif // console.h

