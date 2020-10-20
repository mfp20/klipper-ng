#ifndef UTILITY_MACROS_H
#define UTILITY_MACROS_H

// creates a bit mask. note: the compiler will truncate the value to 16-bits max.
#define BIT(x) (0x01 << (x))
// creates a bit mask for unsigned long (32 bit). 
#define BITLONG(x) ((uint32_t)0x00000001 << (x))
// to get bit number 3 of 'foo', bit_get(foo, BIT(3))
#define BIT_GET(p,m) ((p) & (m))
// to set bit number 3 of 'foo', bit_set(foo, BIT(3))
#define BIT_SET(p,m) ((p) |= (m))
// to clear bit number 3 of 'foo', bit_clear(foo, BIT(3))
#define BIT_CLEAR(p,m) ((p) &= ~(m))
// to flip bit number 3 of 'foo', bit_flip(foo, BIT(3))
#define BIT_FLIP(p,m) ((p) ^= (m))
// to set or clear bar's bit 0, depending on foo's bit 4, bit_write(bit_get(foo, BIT(4)), bar, BIT(0))
#define BIT_WRITE(c,p,m) (c ? BIT_SET(p,m) : BIT_CLEAR(p,m))

// returns the uint16_t's MSB
#define MSB16(a) \
	({ \
	 __typeof__ (a) _a = (a); \
	 _a & 0xF0; \
	 })

// returns the uint16_t's LSB
#define LSB16(a) \
	({ \
	 __typeof__ (a) _a = (a); \
	 _a & 0x0F; \
	 })

// returns the max of 2 values
#define MAX(a,b) \
	({ \
	 __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; \
	 })

#endif

