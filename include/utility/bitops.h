
#ifndef UTILITY_BITOPS_H
#define UTILITY_BITOPS_H

	// creates a bit mask. note: the compiler will truncate the value to 16-bits max.
#define BIT(x) (0x01 << (x))
	// creates a bit mask for unsigned long (32 bit). 
#define BITLONG(x) ((unsigned long)0x00000001 << (x))
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

#endif

