#include <fixmath/fix32.h>
//#include "int64.h"


/* Subtraction and addition with overflow detection.
 * The versions without overflow detection are inlined in the header.
 */
#ifndef FIXMATH_NO_OVERFLOW
fix32_t fix32_add(fix32_t a, fix32_t b)
{
	// Use unsigned integers because overflow with signed integers is
	// an undefined operation (http://www.airs.com/blog/archives/120).
	uint64_t _a = a, _b = b;
	uint64_t sum = _a + _b;

	// Overflow can only happen if sign of a == sign of b, and then
	// it causes sign of sum != sign of a.
	if (!((_a ^ _b) & 0x8000000000000000ULL) && ((_a ^ sum) & 0x8000000000000000ULL))
		return fix32_overflow;

	return sum;
}

fix32_t fix32_sub(fix32_t a, fix32_t b)
{
	uint64_t _a = a, _b = b;
	uint64_t diff = _a - _b;

	// Overflow can only happen if sign of a != sign of b, and then
	// it causes sign of diff != sign of a.
	if (((_a ^ _b) & 0x8000000000000000ULL) && ((_a ^ diff) & 0x8000000000000000ULL))
		return fix32_overflow;

	return diff;
}

/* Saturating arithmetic */
fix32_t fix32_sadd(fix32_t a, fix32_t b)
{
	fix32_t result = fix32_add(a, b);

	if (result == fix32_overflow)
		return (a >= 0) ? fix32_maximum : fix32_minimum;

	return result;
}

fix32_t fix32_ssub(fix32_t a, fix32_t b)
{
	fix32_t result = fix32_sub(a, b);

	if (result == fix32_overflow)
		return (a >= 0) ? fix32_maximum : fix32_minimum;

	return result;
}
#endif


/* 64-bit implementation of fix32_mul. Potentially fast on 32-bit processors,
 * and this is a relatively good compromise for compilers that do not support
 * uint128_t. Uses 32*32->64bit multiplications.
 */
fix32_t fix32_mul(fix32_t inArg0, fix32_t inArg1)
{
	// Each argument is divided to 32-bit parts.
	//					AB
	//			*	 CD
	// -----------
	//					BD	32 * 32 -> 64 bit products
	//				 CB
	//				 AD
	//				AC
	//			 |----| 128 bit product
	int64_t A = (inArg0 >> 32), C = (inArg1 >> 32);
	uint64_t B = (inArg0 & 0xFFFFFFFF), D = (inArg1 & 0xFFFFFFFF);

	int64_t AC = A*C;
	int64_t AD_CB = A*D + C*B;
	uint64_t BD = B*D;

	int64_t product_hi = AC + (AD_CB >> 32);

	// Handle carry from lower 64 bits to upper part of result.
	uint64_t ad_cb_temp = AD_CB << 32;
	uint64_t product_lo = BD + ad_cb_temp;
	if (product_lo < BD)
		product_hi++;

#ifndef FIXMATH_NO_OVERFLOW
	// The upper 33 bits should all be the same (the sign).
	if (product_hi >> 63 != product_hi >> 31)
		return fix32_overflow;
#endif

#ifdef FIXMATH_NO_ROUNDING
	return (product_hi << 32) | (product_lo >> 32);
#else
	// Subtracting 0x80000000 (= 0.5) and then using signed right shift
	// achieves proper rounding to result-1, except in the corner
	// case of negative numbers and lowest word = 0x80000000.
	// To handle that, we also have to subtract 1 for negative numbers.
	uint64_t product_lo_tmp = product_lo;
	product_lo -= 0x80000000;
	product_lo -= (uint64_t)product_hi >> 63;
	if (product_lo > product_lo_tmp)
		product_hi--;

	// Discard the lowest 32 bits. Note that this is not exactly the same
	// as dividing by 0x100000000. For example if product = -1, result will
	// also be -1 and not 0. This is compensated by adding +1 to the result
	// and compensating this in turn in the rounding above.
	fix32_t result = (product_hi << 32) | (product_lo >> 32);
	result += 1;
	return result;
#endif
}


#ifndef FIXMATH_NO_OVERFLOW
/* Wrapper around fix32_mul to add saturating arithmetic. */
fix32_t fix32_smul(fix32_t inArg0, fix32_t inArg1)
{
	fix32_t result = fix32_mul(inArg0, inArg1);

	if (result == fix32_overflow)
	{
		if ((inArg0 >= 0) == (inArg1 >= 0))
			return fix32_maximum;
		else
			return fix32_minimum;
	}

	return result;
}
#endif

/* 64-bit implementation of fix32_div. Fastest version for e.g. ARM Cortex M3.
 * Performs 64-bit divisions repeatedly to reduce the remainder. For this to
 * be efficient, the processor has to have 64-bit hardware division.
 */
#ifdef __GNUC__
// Count leading zeros, using processor-specific instruction if available.
#define clz(x) (__builtin_clzll(x) - (8 * sizeof(long long) - 64))
#else
static uint8_t clz(uint64_t x)
{
	uint8_t result = 0;
	if (x == 0) return 64;
	while (!(x & 0xF000000000000000)) { result += 4; x <<= 4; }
	while (!(x & 0x8000000000000000)) { result += 1; x <<= 1; }
	return result;
}
#endif

fix32_t fix32_div(fix32_t a, fix32_t b)
{
	// This uses a hardware 64/64 bit division multiple times, until we have
	// computed all the bits in (a<<33)/b. Usually this takes 1-3 iterations.

	if (b == 0)
			return fix32_minimum;

	uint64_t remainder = (a >= 0) ? a : (-a);
	uint64_t divider = (b >= 0) ? b : (-b);
	uint64_t quotient = 0;
	int bit_pos = 33;

	// Kick-start the division a bit.
	// This improves speed in the worst-case scenarios where N and D are large
	// It gets a lower estimate for the result by N/(D >> 33 + 1).
	if (divider & 0xFFFFFFF000000000ULL)
	{
		uint64_t shifted_div = ((divider >> 33) + 1);
		quotient = remainder / shifted_div;

		// Implement this:		remainder -= ((uint128_t)quotient * divider) >> 33;

		int64_t A = (quotient >> 32), C = (divider >> 32);
		uint64_t B = (quotient & 0xFFFFFFFF), D = (divider & 0xFFFFFFFF);

		int64_t AC = A*C;
		int64_t AD_CB = A*D + C*B;
		uint64_t BD = B*D;

		int64_t product_hi = AC + (AD_CB >> 32);

		// Handle carry from lower 64 bits to upper part of result.
		uint64_t ad_cb_temp = AD_CB << 32;
		uint64_t product_lo = BD + ad_cb_temp;
		if (product_lo < BD)
			product_hi++;

		remainder -= (product_hi << (64-33)) | (product_lo >> 33);
	}

	// If the divider is divisible by 2^n, take advantage of it.
	while (!(divider & 0xF) && bit_pos >= 4)
	{
		divider >>= 4;
		bit_pos -= 4;
	}

	while (remainder && bit_pos >= 0)
	{
		// Shift remainder as much as we can without overflowing
		int shift = clz(remainder);
		if (shift > bit_pos) shift = bit_pos;
		remainder <<= shift;
		bit_pos -= shift;

		uint64_t div = remainder / divider;
		remainder = remainder % divider;
		quotient += div << bit_pos;

		#ifndef FIXMATH_NO_OVERFLOW
		if (div & ~(0xFFFFFFFFFFFFFFFFULL >> bit_pos))
				return fix32_overflow;
		#endif

		remainder <<= 1;
		bit_pos--;
	}

	#ifndef FIXMATH_NO_ROUNDING
	// Quotient is always positive so rounding is easy
	quotient++;
	#endif

	fix32_t result = quotient >> 1;

	// Figure out the sign of the result
	if ((a ^ b) & 0x8000000000000000ULL)
	{
		#ifndef FIXMATH_NO_OVERFLOW
		if (result == fix32_minimum)
				return fix32_overflow;
		#endif

		result = -result;
	}

	return result;
}

#ifndef FIXMATH_NO_OVERFLOW
/* Wrapper around fix32_div to add saturating arithmetic. */
fix32_t fix32_sdiv(fix32_t inArg0, fix32_t inArg1)
{
	fix32_t result = fix32_div(inArg0, inArg1);

	if (result == fix32_overflow)
	{
		if ((inArg0 >= 0) == (inArg1 >= 0))
			return fix32_maximum;
		else
			return fix32_minimum;
	}

	return result;
}
#endif

fix32_t fix32_mod(fix32_t x, fix32_t y)
{
	#ifdef FIXMATH_OPTIMIZE_8BIT
		/* The reason we do this, rather than use a modulo operator
		 * is that if you don't have a hardware divider, this will result
		 * in faster operations when the angles are close to the bounds.
		 */
		while(x >=  y) x -= y;
		while(x <= -y) x += y;
	#else
		/* Note that in C90, the sign of result of the modulo operation is
		 * undefined. in C99, it's the same as the dividend (aka numerator).
		 */
		x %= y;
	#endif

	return x;
}
