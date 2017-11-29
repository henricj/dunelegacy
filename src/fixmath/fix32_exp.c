#include <fixmath/fix32.h>
#include <stdbool.h>

#ifndef FIXMATH_NO_CACHE
static fix32_t _fix32_exp_cache_index[4096]  = { 0 };
static fix32_t _fix32_exp_cache_value[4096]  = { 0 };
#endif


// TODO: rapidly looses accuracy for inValue > 7.0
fix32_t fix32_exp(fix32_t inValue) {
	if(inValue == 0        ) return fix32_one;
	if(inValue == fix32_one) return fix32_e;
	if(inValue >= 0x000000157CD0E703LL   ) return fix32_maximum;	// ln(2^31)
	if(inValue <= -0x00000016DFB516F2LL  ) return 0;	// ln(2^-33)

	#ifndef FIXMATH_NO_CACHE
	fix32_t tempIndex = (inValue ^ (inValue >> 16));
	tempIndex = (inValue ^ (inValue >> 4)) & 0x0FFF;
	if(_fix32_exp_cache_index[tempIndex] == inValue)
		return _fix32_exp_cache_value[tempIndex];
	#endif

	/* The algorithm is based on the power series for exp(x):
	 * http://en.wikipedia.org/wiki/Exponential_function#Formal_definition
	 *
	 * From term n, we get term n+1 by multiplying with x/n.
	 * When the sum term drops to zero, we can stop summing.
	 */

	// The power-series converges much faster on positive values
	// and exp(-x) = 1/exp(x).
	bool neg = (inValue < 0);
	if (neg) inValue = -inValue;

	fix32_t result = inValue + fix32_one;
	fix32_t term = inValue;

	uint_fast8_t i;
	for (i = 2; i < 60; i++)
	{
		term = fix32_mul(term, fix32_div(inValue, fix32_from_int(i)));
		result += term;

		if ((term < 500) && ((i > 60) || (term < 20)))
			break;
	}

	if (neg) result = fix32_div(fix32_one, result);

	#ifndef FIXMATH_NO_CACHE
	_fix32_exp_cache_index[tempIndex] = inValue;
	_fix32_exp_cache_value[tempIndex] = result;
	#endif

	return result;
}


// TODO: rapidly looses accuracy for inValue > 20.0
fix32_t fix32_log(fix32_t inValue)
{
	fix32_t guess = fix32_from_int(2);
	fix32_t delta;
	int scaling = 0;
	int count = 0;

	if (inValue <= 0)
		return fix32_minimum;

	// Bring the value to the most accurate range (1 < x < 100)
	const fix32_t e_to_fourth = 0x0000003699205C4EULL;	// e^4
	while (inValue > fix32_from_int(100))
	{
		inValue = fix32_div(inValue, e_to_fourth);
		scaling += 4;
	}

	while (inValue < fix32_one)
	{
		inValue = fix32_mul(inValue, e_to_fourth);
		scaling -= 4;
	}

	do
	{
		// Solving e(x) = y using Newton's method
		// f(x) = e(x) - y
		// f'(x) = e(x)
		fix32_t e = fix32_exp(guess);
		delta = fix32_div(inValue - e, e);

		// It's unlikely that logarithm is very large, so avoid overshooting.
		if (delta > fix32_from_int(3))
			delta = fix32_from_int(3);

		guess += delta;
	} while ((count++ < 20)
		&& ((delta > 1) || (delta < -1)));

	return guess + fix32_from_int(scaling);
}



static inline fix32_t fix32_rs(fix32_t x)
{
	#ifdef FIXMATH_NO_ROUNDING
		return (x >> 1);
	#else
		fix32_t y = (x >> 1) + (x & 1);
		return y;
	#endif
}

/**
 * This assumes that the input value is >= 1.
 *
 * Note that this is only ever called with inValue >= 1 (because it has a wrapper to check.
 * As such, the result is always less than the input.
 */
static fix32_t fix32__log2_inner(fix32_t x)
{
	fix32_t result = 0;

	while(x >= fix32_from_int(2))
	{
		result++;
		x = fix32_rs(x);
	}

	if(x == 0) return (result << 32);

	uint_fast8_t i;
	for(i = 32; i > 0; i--)
	{
		x = fix32_mul(x, x);
		result <<= 1;
		if(x >= fix32_from_int(2))
		{
			result |= 1;
			x = fix32_rs(x);
		}
	}
	#ifndef FIXMATH_NO_ROUNDING
		x = fix32_mul(x, x);
		if(x >= fix32_from_int(2)) result++;
	#endif

	return result;
}



/**
 * calculates the log base 2 of input.
 * Note that negative inputs are invalid! (will return fix32_overflow, since there are no exceptions)
 *
 * i.e. 2 to the power output = input.
 * It's equivalent to the log or ln functions, except it uses base 2 instead of base 10 or base e.
 * This is useful as binary things like this are easy for binary devices, like modern microprocessros, to calculate.
 *
 * This can be used as a helper function to calculate powers with non-integer powers and/or bases.
 */
fix32_t fix32_log2(fix32_t x)
{
	// Note that a negative x gives a non-real result.
	// If x == 0, the limit of log2(x)  as x -> 0 = -infinity.
	// log2(-ve) gives a complex result.
	if (x <= 0) return fix32_overflow;

	// If the input is less than one, the result is -log2(1.0 / in)
	if (x < fix32_one)
	{
		// Note that the inverse of this would overflow.
		// This is the exact answer for log2(1.0 / 4294967296)
		if (x == 1) return fix32_from_int(-32);

		fix32_t inverse = fix32_div(fix32_one, x);
		return -fix32__log2_inner(inverse);
	}

	// If input >= 1, just proceed as normal.
	// Note that x == fix32_one is a special case, where the answer is 0.
	return fix32__log2_inner(x);
}

/**
 * This is a wrapper for fix32_log2 which implements saturation arithmetic.
 */
fix32_t fix32_slog2(fix32_t x)
{
	fix32_t retval = fix32_log2(x);
	// The only overflow possible is when the input is negative.
	if(retval == fix32_overflow)
		return fix32_minimum;
	return retval;
}
