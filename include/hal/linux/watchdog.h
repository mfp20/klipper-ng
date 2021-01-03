#ifndef __LINUX_WATCHDOG_H
#define __LINUX_WATCHDOG_H

int watchdog_setup(void);
void task_watchdog(void);

#endif // watchdog.h

