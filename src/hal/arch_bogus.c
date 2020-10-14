
#include <hal/arch.h>
#include <x86intrin.h>	// rdtscp
#include <stdarg.h>		// va_arg
#include <unistd.h>		// usleep
#include <pthread.h>	// timer thread
#include <termios.h>	// termios
#include <pty.h>		// openpty
#include <fcntl.h>		// fcntl
#include <sys/ioctl.h>	//ioctl
#include <sys/stat.h>	// chmod
#include <stdio.h>		// sprintf
#include <string.h>		// memset, strcopy

// TIME -----------------------------------------------------------------------------------
volatile uint16_t tnano = 0;
volatile uint16_t tmicro = 0;
volatile uint16_t tmilli = 0;
volatile uint16_t tsecond = 0;

static uint64_t ticks() {
	unsigned bogo;
	return __rdtscp(&bogo); // read intel TSC cycles counter
}

static uint64_t ns() {
	static uint64_t is_init = 0;
	static struct timespec linux_rate;
	if (0 == is_init) {
		clock_getres(CLOCK_MONOTONIC, &linux_rate);
		is_init = 1;
	}
	uint64_t now;
	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
	return now;
}

static pthread_t timer_thread_id;
static pthread_mutex_t timer_lock;
static uint16_t timer_dummyload_1us = 540;
static double timer_cdiff=4000, timer_ndiff=1000, timer_avg=4.20;
static void * _timer_thread(void * data) {
	int cstart, cend, bogo = 0;
	uint64_t nstart, nend;
	while(1) {
		cstart = ticks();
		nstart = ns();
		// set new time values
		pthread_mutex_lock(&timer_lock);
		tnano = tnano + ((uint16_t)(timer_cdiff/timer_avg));
		tmicro = tmicro + tnano/1000;
		tmilli = tmilli + tmicro/1000;
		tsecond = tsecond + tmilli/1000;
		tmilli = tmilli % 1000;
		tmicro = tmicro % 1000;
		tnano = tnano % 1000;
		pthread_mutex_unlock(&timer_lock);
		//
		for (int i = 0;i<timer_dummyload_1us;i++) bogo+=bogo;
		nend = ns();
		cend = ticks();
		timer_ndiff = nend-nstart;
		timer_cdiff = cend-cstart;
		pthread_mutex_lock(&timer_lock);
		timer_avg = (timer_avg+(timer_cdiff/timer_ndiff))/2;
		pthread_mutex_unlock(&timer_lock);
	}
	data = 0; // unused parameter
	free(data);
	return 0;
}

static int timer_init() {
	pthread_mutex_init(&timer_lock, NULL);
	if(pthread_create(&timer_thread_id, NULL, _timer_thread, NULL)) return 0;
	return 1;
}

static void timer_clean() {
	pthread_cancel(timer_thread_id);
	pthread_join(timer_thread_id, NULL);
	pthread_mutex_destroy(&timer_lock);
}

uint16_t nanos(void) {
	pthread_mutex_lock(&timer_lock);
	uint16_t t = tnano;
	pthread_mutex_unlock(&timer_lock);
	return t;
}

uint16_t micros(void) {
	pthread_mutex_lock(&timer_lock);
	uint16_t t = tmicro;
	pthread_mutex_unlock(&timer_lock);
	return t;
}

uint16_t millis(void) {
	pthread_mutex_lock(&timer_lock);
	uint16_t t = tmilli;
	pthread_mutex_unlock(&timer_lock);
	return t;
}

uint16_t seconds(void) {
	pthread_mutex_lock(&timer_lock);
	uint16_t t = tsecond;
	pthread_mutex_unlock(&timer_lock);
	return t;
}


// PIN GET/SET -------------------------------------------------------------------------------------------
void pin_mode(uint8_t pin, uint8_t mode) {
	usleep(1);
}

uint8_t pin_read(uint8_t pin) {
	return rand() % 1;
}

void pin_set_low(uint8_t pin) {
	usleep(1);
}

void pin_set_high(uint8_t pin) {
	usleep(1);
}

void pin_toggle(uint8_t pin) {
	usleep(1);
}

void pin_write(uint8_t pin, uint8_t value) {
	usleep(1);
}

void pin_pullup_enable(uint8_t pin) {
	usleep(1);
}

void pin_open_drain(uint8_t pin) {
	usleep(1);
}

unsigned char pin_port_read(uint8_t port, uint8_t bitmask) {
	unsigned char out = 0;
	// TODO
	//if (port == 0) return (PIND & 0xFC) & bitmask; // ignore Rx/Tx 0/1
	//if (port == 1) return ((PINB & 0x3F) | ((PINC & 0x03) << 6)) & bitmask;
	//if (port == 2) return ((PINC & 0x3C) >> 2) & bitmask;
	return 0;
	return out;
}

unsigned char pin_port_write(uint8_t port, uint8_t value, uint8_t bitmask) {
	if (port == 0) {
		//bitmask = bitmask & 0xFC;  // do not touch Tx & Rx pins
		//uint8_t valD = value & bitmask;
		//uint8_t maskD = ~bitmask;
		//cli();
		//PORTD = (PORTD & maskD) | valD;
		//sei();
	} else if (port == 1) {
		//uint8_t valB = (value & bitmask) & 0x3F;
		//uint8_t valC = (value & bitmask) >> 6;
		//uint8_t maskB = ~(bitmask & 0x3F);
		//uint8_t maskC = ~((bitmask & 0xC0) >> 6);
		//cli();
		//PORTB = (PORTB & maskB) | valB;
		//PORTC = (PORTC & maskC) | valC;
		//sei();
	} else if (port == 2) {
		//bitmask = bitmask & 0x0F;
		//uint8_t valC = (value & bitmask) << 2;
		//uint8_t maskC = ~(bitmask << 2);
		//cli();
		//PORTC = (PORTC & maskC) | valC;
		//sei();
	}
	return 1;
}


// ADC -----------------------------------------------------------------------------------------------
volatile uint8_t adc_enable[16];
volatile uint8_t adc_value[16];
volatile uint8_t adc_current = 0;

static void pin_input_adc_run(void) {
	if (adc_enable[adc_current]) {
		adc_value[adc_current] = rand() % 256;
	}
	if (adc_current < 15) {
		adc_current++;
	} else {
		adc_current = 0;
	}
}


// PULSING -----------------------------------------------------------------------------------------------
volatile pin_pulsing_t pulsing;

uint16_t pin_pulse_detect(uint8_t pin) {
	uint16_t width = rand() % 1000;
	usleep(width);
	return width;
}

void pin_pulse_single(uint8_t pin, uint16_t width) {
	usleep(width);
}

volatile pin_frame_t* pin_pulse_multi(uint16_t milli) {
	if (pulsing.head) {
		if (milli > pulsing.tail->milli) {
			pulsing.tail->next = (volatile pin_frame_t*)malloc(sizeof(pin_frame_t));
			pulsing.tail->next->next = NULL;
			pulsing.tail = pulsing.tail->next;
			return pulsing.tail->next;
		} else {
			// TODO: traverse all list from head to find the right timing spot
			return pulsing.tail->next;
		}
	} else {
		pulsing.head = (volatile pin_frame_t*)malloc(sizeof(pin_frame_t));
		pulsing.head->next = NULL;
		pulsing.tail = pulsing.head;
		return pulsing.tail;
	}
}


// COMMS -----------------------------------------------------------------------------------
commport_t *ports = NULL;
uint8_t ports_no = 0;
commport_t *console = NULL;

commport_t* commport_register(uint8_t type, uint8_t no) {
	// allocate ports array
	ports = (commport_t *)realloc(ports, sizeof(commport_t)*COMMPORT_QTY);
	// assign metods
	ports[ports_no].type = type;
	ports[ports_no].no = no;
	switch (type) {
		case COMMPORT_TYPE_1WIRE:
			//ports[ports_no].begin = onewire_begin;
			//ports[ports_no].available = onewire_available;
			//ports[ports_no].read = onewire_read;
			//ports[ports_no].write = onewire_write;
			//ports[ports_no].end = onewire_end;
			break;
		case COMMPORT_TYPE_UART:
			ports[ports_no].fd = 0;
			ports[ports_no].baud = DEFAULT_BAUD;
			ports[ports_no].begin = uart_begin;
			ports[ports_no].available = uart_available;
			ports[ports_no].read = uart_read;
			ports[ports_no].write = uart_write;
			ports[ports_no].end = uart_end;
			break;
		case COMMPORT_TYPE_I2C:
			//ports[ports_no].begin = i2c_begin;
			//ports[ports_no].available = i2c_available;
			//ports[ports_no].read = i2c_read;
			//ports[ports_no].write = i2c_write;
			//ports[ports_no].end = i2c_end;
			break;
		case COMMPORT_TYPE_SPI:
			//ports[ports_no].begin = spi_begin;
			//ports[ports_no].available = spi_available;
			//ports[ports_no].read = spi_read;
			//ports[ports_no].write = spi_write;
			//ports[ports_no].end = spi_end;
			break;
	}
	if (ports_no == 0) console = &ports[ports_no];
	ports_no++;
	return &ports[ports_no-1];
}

void consoleWrite(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf (buffer,format, args);
	write(console->fd, buffer, len);
	va_end (args);
}

void logWrite(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf (buffer,format, args);
	write(1, buffer, len); // 1 = stdout
	va_end (args);
}

void errWrite(const char *format, ...) {
	char buffer[256];
	va_list args;
	va_start (args, format);
	int len = vsprintf (buffer,format, args);
	write(2, buffer, len); // 2 = stderr
	va_end (args);
}


// UART
static const char *uart_pty_filename = "/tmp/klipper-ng-pty";
static uint8_t uart_pty_count = 0;
int uart_begin(commport_t *uart, uint32_t baud) {
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
	// create pty filename
	char fnid[3];
	sprintf(fnid, "%d", uart_pty_count);
	uart_pty_count++;
	uint8_t len = strlen(uart_pty_filename)+3;
	char filename[len];
	strcpy(filename, uart_pty_filename);
	strcat(filename, fnid);
	// create symlink to tty
	unlink(filename);
	char *tname = ttyname(sfd);
	if (symlink(tname, filename)) printf("symlink error");
	chmod(tname, 0660);
	// make sure stderr is non-blocking
	flags = fcntl(STDERR_FILENO, F_GETFL);
	fcntl(STDERR_FILENO, F_SETFL, flags | O_NONBLOCK);
	// register fd&speed in local ports array
	uart->fd = mfd;
	uart->baud = baud;
	//
	logWrite("PTY ready @ %s\n", filename);
	return mfd;
}

bool uart_available(commport_t *uart) {
	//struct timeval tv;
	//tv.tv_sec = 0;
    //tv.tv_usec = 5;
	//select(NULL, &uart->fd, NULL, NULL, &tv);
	int count;
	ioctl(uart->fd, FIONREAD, &count);
	return (bool)count;
}

int uart_read(commport_t *uart, uint8_t *data, uint8_t count, uint16_t timeout) {
	read(uart->fd, data, count);
	printf(">> ");
	for(int i=0;i<count;i++) {
		printf("%0x ", data[i]);
	}
	printf("\n");
	return 0;
}

int uart_write(commport_t *uart, uint8_t *data, uint8_t count, uint16_t timeout) {
	write(uart->fd, data, count);
	printf("<< ");
	for(int i=0;i<count;i++) {
		printf("%0x ", data[i]);
	}
	printf("\n");
	return 0;
}

int uart_end(commport_t *uart) {
	// TODO close uart->fd
	uart_pty_count--;
	return 0;
}

void _arch_init(void) {
	srand(time(NULL));
	timer_init();
	// init default port (ports[0])
	commport_register(COMMPORT_TYPE_UART, 0);
	ports[0].begin(&ports[0], DEFAULT_BAUD);
	//printf("_arch_init() OK\n");
}

void _arch_run(void) {
	pin_input_adc_run();
}

void _arch_reset(void) {
	timer_clean();
}

