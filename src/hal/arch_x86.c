
#include <hal/arch.h>
#include <math.h> // sqrt
#include <pthread.h> // timer thread
#include <x86intrin.h>  // rdtscp
#include <termios.h>
#include <pty.h> // openpty
#include <sys/stat.h> // chmod

/*
// amount of primes below 'n'
static int dummyload(int n) {
	int i,j;
	int freq=n-1;
	for (i=2; i<=n; ++i) for (j=sqrt(i);j>1;--j) if (i%j==0) {--freq; break;}
	return freq;
}
*/

// TIME -----------------------------------------------------------------------------------
volatile uint16_t tnano;
volatile uint16_t tmicro;
volatile uint16_t tmilli;
volatile uint16_t tsecond;

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
	int cstart, cend, bogo;
	uint64_t nstart, nend;
	while(1) {
		cstart = ticks();
		nstart = ns();
		pthread_mutex_lock(&timer_lock);
		tnano = tnano + ((uint16_t)(timer_cdiff/timer_avg));
		tmicro = tmicro + tnano/1000;
		tmilli = tmilli + tmicro/1000;
		tsecond = tsecond + tmilli/1000;
		tmilli = tmilli % 1000;
		tmicro = tmicro % 1000;
		tnano = tnano % 1000;
		pthread_mutex_unlock(&timer_lock);
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

/*
static double timer_perf(uint8_t print) {
	pthread_mutex_lock(&timer_lock);
	double avg = timer_avg;
	pthread_mutex_unlock(&timer_lock);
	if (print) {
		printf("%lf cycles/ns (avg)\n", avg);
	}
	return avg;
}
*/

static void timer_clean() {
    pthread_cancel(timer_thread_id);
    pthread_join(timer_thread_id, NULL);
    pthread_mutex_destroy(&timer_lock);
}

/*
static uint64_t nanostamp(uint8_t print) {
	uint64_t nanostamp;
	pthread_mutex_lock(&timer_lock);
	nanostamp = (uint64_t)tsecond*1000000000+(uint64_t)tmilli*1000000+(uint64_t)tmicro*1000+(uint64_t)tnano;
	pthread_mutex_unlock(&timer_lock);
	if (print) {
		printf("%20lu ns\n", nanostamp);
	}
	return nanostamp;
}
*/

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

static void time_print() {
	printf("%3d:%3d:%3d:%3d ", seconds(), millis(), micros(), nanos());
}


// PIN GET/SET -------------------------------------------------------------------------------------------

void pin_mode(uint8_t pin, uint8_t mode) {
	pin = pin;
	mode = mode;
	usleep(1);
}
uint8_t pin_read(uint8_t pin) {
	pin = pin;
	return rand() % 1;
}
void pin_set_low(uint8_t pin) {
	pin = pin;
	usleep(1);
}
void pin_set_high(uint8_t pin) {
	pin = pin;
	usleep(1);
}
void pin_toggle(uint8_t pin) {
	pin = pin;
	usleep(1);
}
void pin_write(uint8_t pin, uint8_t value) {
	pin = pin;
	value = value;
	usleep(1);
}
void pin_pullup_enable(uint8_t pin) {
	pin = pin;
	usleep(1);
}
void pin_open_drain(uint8_t pin) {
	pin = pin;
	usleep(1);
}


// ADC -----------------------------------------------------------------------------------------------
volatile uint8_t adc_enable[16];
volatile uint8_t adc_value[16];
volatile uint8_t adc_current;

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
	pin = pin;
	uint16_t width = rand() % 1000;
	usleep(width);
	return width;
}

void pin_pulse_single(uint8_t pin, uint16_t width) {
	pin = pin;
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
commport_t *commports[8];

// UART0
static const char *uart0_filename = "/tmp/klipper-pty0";
uint8_t uart_enable(commport_t *uart, uint32_t baud) {
	free(uart);
	baud = baud;
	// Open pseudo-tty
	struct termios ti;
	memset(&ti, 0, sizeof(ti));
	int mfd, sfd;
	openpty(&mfd, &sfd, NULL, &ti, NULL);
	//fcntl(mfd, F_SETFD, FD_CLOEXEC);
	//fcntl(sfd, F_SETFD, FD_CLOEXEC);
	// Create symlink to tty
	unlink(uart0_filename);
	char *tname = ttyname(sfd);
	if (symlink(tname, uart0_filename)) printf("symlink error");
	chmod(tname, 0660);
	printf("PTY ready @ %s\n", uart0_filename);
	return mfd;
}

bool uart_need_rx(commport_t *uart) {
	return cbuf_is_empty(&uart->rx_buf);
}

uint8_t uart_rx_byte(commport_t *uart) {
	return read(uart->fd, &uart->rx_buf.data[uart->rx_buf.tail], 1);
}

uint8_t uart_rx_all(commport_t *uart, uint16_t timeout) {
	free(uart);
	timeout = timeout;
	//return read(uart->fd, &uart->rx_buf.data[uart->rx_buf.tail], sizeof(uart->rx_buf.data)-uart->rx_buf.tail);
	return 0;
}

bool uart_need_tx(commport_t *uart) {
	return cbuf_is_empty(&uart->tx_buf);
}

uint8_t uart_tx_byte(commport_t *uart) {
	free(uart);
	//return write(uart->fd, &uart->tx_buf.data[uart->tx_buf.tail], 1);
	return 0;
}

uint8_t uart_tx_all(commport_t *uart, uint16_t timeout) {
	timeout = timeout;
	return write(uart->fd, &uart->tx_buf.data[uart->tx_buf.tail], sizeof(uart->tx_buf.data)-uart->tx_buf.head);
}

void uart_disable(commport_t *uart) {
	free(uart);
}


// ARCH -----------------------------------------------------------------------------------

void logger(const char *string) {
	time_print();
	printf("%s\n", string);
}

void arch_init(void) {
	tnano = 0;
	tmicro = 0;
	tmilli = 0;
	tsecond = 0;
	//
	for (uint8_t i = 0;i<8;i++) {
		pulsing.pin[i] = 0;
	}
	pulsing.head = NULL;
	pulsing.tail = NULL;
	//
	for (uint8_t i = 0;i<8;i++) {
		adc_enable[i] = 0;
		adc_value[i] = 0;
	}
	adc_current = 0;
	//
	srand(time(NULL));
	timer_init();
}

uint8_t arch_run(uint16_t snow, uint16_t mnow, uint16_t utimeout) {
	snow = snow;
	mnow = mnow;
	utimeout = utimeout;
	// read adc(s)
	pin_input_adc_run();
	//
	return 0;
}

uint8_t arch_close(void) {
	timer_clean();
	return 0;
}

