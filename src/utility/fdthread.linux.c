
#include <utility/fdthread.linux.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <termios.h>	// termios
#include <pty.h>		// openpty

#define MAX_EVENTS 64

const char *fd_pty_filename = "/tmp/klipper-ng-pty";
int fd_idx = 3;
static fd_t *fds;
static int epoll_fd, running = 1, fdn = 0;
static pthread_mutex_t fds_lock, rx_lock, tx_lock;
static pthread_t epoll_th;

fptr_alarm_t rBufferFullHandler;
fptr_alarm_t wBufferFullHandler;

static void *_epoll_th(void *data) {
	int event_count, i;
	size_t bytes_read;
	struct epoll_event events[MAX_EVENTS];
	while(running) {
		//printf("epoll start\n");
		// read polled fds
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 0);
		pthread_mutex_lock(&fds_lock);
		pthread_mutex_lock(&rx_lock);
		for(i = 0; i < event_count; i++) {
			int idx;
			for(idx=0;idx<fdn;idx++) if(fds[fdn].event.data.fd==events[i].data.fd) break;
			if (fds[idx].rx_buffer_len<BLOCK_SIZE) {
				bytes_read = read(events[i].data.fd, (&fds[idx].rx_buffer)+fds[idx].rx_buffer_len, BLOCK_SIZE-fds[idx].rx_buffer_len);
				fds[idx].rx_buffer_len += bytes_read;
			} else {
				if ((fds[idx].rx_buffer_len >= BLOCK_SIZE)&&(rBufferFullHandler!=NULL)) rBufferFullHandler(idx);
			}
		}
		pthread_mutex_unlock(&rx_lock);
		int n = fdn, i = 0;
		while (i<n) {
			//printf("%d FDs, FD %d\n", n, i);
			// read file fds
			//printf("fds[%d].rx_buffer_len %d\n", i, fds[i].rx_buffer_len);
			pthread_mutex_lock(&rx_lock);
			if(fds[i].type == FD_TYPE_FILE||FD_TYPE_PTY) {
				if (fds[i].rx_buffer_len<BLOCK_SIZE) {
					printf("read %d, len %d\n", i, fds[i].rx_buffer_len);
					int res = read(events[i].data.fd, (&fds[i].rx_buffer)+fds[i].rx_buffer_len, BLOCK_SIZE-fds[i].rx_buffer_len);
					//printf("done %d\n", i);
					if(res < 0) {
						printf("Can't read fdid %d (%s)\n", i, strerror(errno));
					} else {
						fds[i].rx_buffer_len += bytes_read;
					}
				}
			}
			pthread_mutex_unlock(&rx_lock);
			// write fds
			//printf("fds[%d].tx_buffer_len %d\n", i, fds[i].tx_buffer_len);
			pthread_mutex_lock(&tx_lock);
			if (fds[i].tx_buffer_len>0) {
				write(fds[i].event.data.fd, fds[i].tx_buffer, fds[i].tx_buffer_len);
				fds[i].tx_buffer_len = 0;
			}
			pthread_mutex_unlock(&tx_lock);
			i++;
		}
		pthread_mutex_unlock(&fds_lock);
		usleep(1);
		//printf("epoll stop\n");
	}
	return NULL;
}

int initFdThread(void) {
	// init data store
	fds = calloc(1, sizeof(fd_t));
	// init epoll
	epoll_fd = epoll_create1(0);
	if(epoll_fd < 0) {
		return epoll_fd;
	}
	// init thread
	pthread_mutex_init(&fds_lock, NULL);
	pthread_mutex_init(&rx_lock, NULL);
	pthread_mutex_init(&tx_lock, NULL);
	int ret = pthread_create(&epoll_th, NULL, _epoll_th, NULL);
	if(ret < 0) return ret;
	return 0;
}

int fdCreateFile(const char *fname, uint8_t type) {
	FILE *fp = fopen(fname, "ab+");
	fclose(fp);
	return 0;
}

int fdCreatePty(const char *fname, uint8_t type) {
	// init termios
	struct termios ti;
	memset(&ti, 0, sizeof(ti));
	// open pseudo-tty
	int mfd, sfd;
	openpty(&mfd, &sfd, NULL, &ti, NULL);
	// set non-blocking
	int flags = fcntl(mfd, F_GETFL);
	fcntl(mfd, F_SETFL, flags | O_NONBLOCK);
	// set close on exec
	fcntl(mfd, F_SETFD, FD_CLOEXEC);
	fcntl(sfd, F_SETFD, FD_CLOEXEC);
	// create symlink to tty
	unlink(fname);
	char *tname = ttyname(sfd);
	if (symlink(tname, fname)) printf("symlink error");
	chmod(tname, 0660);
	// make sure stderr is non-blocking
	flags = fcntl(STDERR_FILENO, F_GETFL);
	fcntl(STDERR_FILENO, F_SETFL, flags | O_NONBLOCK);
	//
	return mfd;
}

int fdCreateSocket(const char *name, uint8_t type) {
	// TODO create
	return 0;
}

int fdOpen(const char *fname, uint8_t type) {
	int newfd = 0;
	// create
	if (access(fname, F_OK) < 0) {
		if (type == FD_TYPE_FILE) fdCreateFile(fname, type);
		else if (type == FD_TYPE_PTY) newfd = fdCreatePty(fname, type);
		else if (type == FD_TYPE_SOCKET) fdCreateSocket(fname, type);
		else return -1;
	}
	// open
	pthread_mutex_lock(&fds_lock);
	fds = realloc(fds, (fdn+1)*sizeof(fd_t));
	pthread_mutex_unlock(&fds_lock);
	fds[fdn].type = type;
	fds[fdn].event.events = EPOLLIN;
	if (newfd) fds[fdn].event.data.fd = newfd;
	else fds[fdn].event.data.fd = open(fname, O_RDWR|O_NONBLOCK);
	printf("- open %s (fd %d)\n", fname, fds[fdn].event.data.fd);
	if(fds[fdn].event.data.fd < 0) {
		printf("Can't open file \"%s\" (%s)\n", fname, strerror(errno));
		return fds[fdn].event.data.fd;
	}
	pthread_mutex_lock(&rx_lock);
	fds[fdn].rx_buffer = calloc(BLOCK_SIZE, sizeof(uint8_t *));
	fds[fdn].rx_buffer_len = 0;
	pthread_mutex_unlock(&rx_lock);
	pthread_mutex_lock(&tx_lock);
	fds[fdn].tx_buffer = calloc(BLOCK_SIZE, sizeof(uint8_t));
	fds[fdn].tx_buffer_len = 0;
	pthread_mutex_unlock(&tx_lock);
	if(type>1) {
		int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fds[fdn].event.data.fd, &fds[fdn].event);
		if(ret) {
			printf("Can't add \"%s\" to poll queue. (%s)\n", fname, strerror(errno));
			return ret;
		}
	}
	pthread_mutex_lock(&fds_lock);
	fdn++;
	pthread_mutex_unlock(&fds_lock);
	return fdn-1;
}

int fdGet(int fdid) {
	return fds[fdid].event.data.fd;
}

int fdAvailable(int fdid) {
	return fds[fdid].rx_buffer_len;
}

int fdRead(int fdid, uint8_t *data) {
	int i = 0;
	if(fds[fdid].type>0) {
		pthread_mutex_lock(&rx_lock);
		while (i<fds[fdid].rx_buffer_len) {
			//printf("fdid %d -> (%d) %d\n",fdid,i,data[i]);
			data[i] = fds[fdid].rx_buffer[i];
			i++;
		}
		fds[fdid].rx_buffer_len = 0;
		pthread_mutex_unlock(&rx_lock);
	} else {
		i = read(fds[fdid].event.data.fd,data,BLOCK_SIZE);
	}
	return i;
}

int fdWrite(int fdid, uint8_t *data, int len) {
	pthread_mutex_lock(&tx_lock);
	if (fds[fdid].tx_buffer_len+len>BLOCK_SIZE) {
		if (wBufferFullHandler!=NULL) wBufferFullHandler(fdid);
		else printf("TX Buffer full.\n");
		pthread_mutex_unlock(&tx_lock);
		return 0;
	}
	int i = 0;
	while (i<len) {
		fds[fdid].tx_buffer[i] = data[i];
		i++;
	}
	fds[fdid].tx_buffer_len += len;
	pthread_mutex_unlock(&tx_lock);
	return len;
}

int fdClose(int fdid) {
	int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, 0, &fds[fdid].event);
	if(ret<0) {
		return ret;
	}
	ret = close(fds[fdid-1].event.data.fd);
	if (ret<0) return ret;
	fdn--;
	return 0;
}

int closeFdThread(void) {
	// flush buffer
	// TODO
	// terminate thread
	pthread_cancel(epoll_th);
	int ret = pthread_join(epoll_th, NULL);
	pthread_mutex_destroy(&fds_lock);
	pthread_mutex_destroy(&rx_lock);
	pthread_mutex_destroy(&tx_lock);
	// close all FDs
	close(epoll_fd);
	while (fdn>0) {
		close(fds[fdn-1].event.data.fd);
		free(fds[fdn-1].rx_buffer);
		free(fds[fdn-1].tx_buffer);
		fdn--;
	}
	free(fds);
	return ret;
}

