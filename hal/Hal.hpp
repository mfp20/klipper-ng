#ifndef HAL_H
#define HAL_H

extern "C" {
	const int TOTAL_PINS = 1;
	void (*delay)(unsigned long ms);
	void (*digitalWrite)(uint8_t pin, uint8_t val);
}

class SerialStream
{
	public:
		virtual void begin(unsigned long baud);
		virtual int available(void);
		virtual int read(void);
		virtual size_t write(uint8_t);
};

extern SerialStream Serial;

#endif

