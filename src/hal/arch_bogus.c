
#include <hal/arch.h>
#include <x86intrin.h>	// rdtscp
#include <unistd.h>		// usleep
#include <pthread.h>	// timer thread
#include <string.h>		// memset
#include <signal.h>		// sigaction
#include <sys/time.h>	// itimerval

// TIME -----------------------------------------------------------------------------------

static uint64_t ticks() {
	unsigned bogo;
	return __rdtscp(&bogo); // read intel TSC cycles counter
}

static pthread_t timer_thread_id;
static pthread_mutex_t timer_lock;
static void _timer_handler(int signum) {
	pthread_mutex_lock(&timer_lock);
	tmicro+=100;
	if(tmicro>=999) {
		tmilli++;
		tmicro=0;
	}
	if(tmilli>=999) {
		tsecond++;
		tmilli=0;
	}
	pthread_mutex_unlock(&timer_lock);
}
static void * _timer_thread(void *data) {
	struct sigaction sa;
	struct itimerval timer;
	// install _timer_handler as the signal handler for SIGALRM.
	memset(&sa, 0, sizeof (sa));
	sa.sa_handler = &_timer_handler;
	sigaction(SIGALRM, &sa, NULL);
	// configure the timer to expire after 1 usec...
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100;
	// ... and every 1 usec after that.
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 100;
	// start the virtual timer (countdown).
	setitimer(ITIMER_REAL, &timer, NULL);
	// loop&do nothing
	while(1) {
		sleep(1);
	}
	data = data; // unused parameter
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

uint16_t cycles(void) {
	return (uint16_t)(ticks() & 0x000000000000FFFF);
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


// PORT GET/SET -----------------------------------------------------------------------------

uint8_t port_read(uint8_t pin, uint8_t timeout) {
	usleep(1);
	return 0;
}

uint8_t port_write(uint8_t pin, uint8_t value) {
	usleep(1);
	return 0;
}


// PIN GET/SET ------------------------------------------------------------------------------

void pin_mode(uint8_t pin, uint8_t mode) {
	usleep(1);
}

uint8_t pin_read(uint8_t pin, uint8_t timeout) {
	return 0;
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

void pin_write_pwm(uint8_t pin, uint16_t value) {
	usleep(1);
}

void pin_pullup_enable(uint8_t pin) {
	usleep(1);
}

void pin_open_drain(uint8_t pin) {
	usleep(1);
}


// ADC -------------------------------------------------------------------------------------

static void pin_adc_run(void) {
	if (pin_adc_enable[pin_adc_current]) {
		pin_adc_value[pin_adc_current] = rand() % 256;
	}
	if (pin_adc_current < 15) {
		pin_adc_current++;
	} else {
		pin_adc_current = 0;
	}
}

uint16_t pin_read_adc(uint8_t pin, uint8_t timeout) {
	return pin_adc_value[pin];
}


// PULSING ----------------------------------------------------------------------------------

uint16_t pin_pulse_detect(uint8_t pin) {
	uint16_t width = rand() % 1000;
	usleep(width);
	return width;
}

void pin_pulse_single(uint8_t pin, uint16_t width) {
	usleep(width);
}

volatile pin_frame_t* pin_pulse_multi(uint16_t milli) {
	if (pin_pulsing.head) {
		if (milli > pin_pulsing.tail->milli) {
			pin_pulsing.tail->next = (volatile pin_frame_t*)calloc(1,sizeof(pin_frame_t));
			pin_pulsing.tail->next->next = NULL;
			pin_pulsing.tail = pin_pulsing.tail->next;
			return pin_pulsing.tail->next;
		} else {
			// TODO: traverse all list from head to find the right timing spot
			return pin_pulsing.tail->next;
		}
	} else {
		pin_pulsing.head = (volatile pin_frame_t*)calloc(1,sizeof(pin_frame_t));
		pin_pulsing.head->next = NULL;
		pin_pulsing.tail = pin_pulsing.head;
		return pin_pulsing.tail;
	}
}

void _arch_init(void) {
	// random init
	srand(time(NULL));
	// timer init
	timer_init();
	// pin init
	pin[0] = P1;
	pin[1] = P2;
	pin[2] = P3;
	pin[3] = P4;
	pin[4] = P5;
	pin[5] = P6;
	pin[6] = P7;
	pin[7] = P8;
}

void _arch_run(void) {
	pin_adc_run();
}

void _arch_reset(void) {
	timer_clean();
}

