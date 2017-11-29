#include <limits.h>
#include <fixmath/fix32.h>

#if !defined(FIXMATH_NO_CACHE)
static fix32_t _fix32_sin_cache_index[4096]  = { 0 };
static fix32_t _fix32_sin_cache_value[4096]  = { 0 };
#endif

#ifndef FIXMATH_NO_CACHE
static fix32_t _fix32_atan_cache_index[2][4096] = { { 0 }, { 0 } };
static fix32_t _fix32_atan_cache_value[4096] = { 0 };
#endif


fix32_t fix32_sin_parabola(fix32_t inAngle)
{
	fix32_t abs_inAngle, abs_retval, retval;
	fix32_t mask;

	/* Absolute function */
	mask = (inAngle >> (sizeof(fix32_t)*CHAR_BIT-1));
	abs_inAngle = (inAngle + mask) ^ mask;

	/* On 0->PI, sin looks like x² that is :
	   - centered on PI/2,
	   - equals 1 on PI/2,
	   - equals 0 on 0 and PI
	  that means :  4/PI * x  - 4/PI² * x²
	  Use abs(x) to handle (-PI) -> 0 zone.
	 */
	retval = fix32_mul(fix32_FOUR_DIV_PI, inAngle) + fix32_mul( fix32_mul(fix32__FOUR_DIV_PI2, inAngle), abs_inAngle );
	/* At this point, retval equals sin(inAngle) on important points ( -PI, -PI/2, 0, PI/2, PI),
	   but is not very precise between these points
	 */
	#ifndef FIXMATH_FAST_SIN
	/* Absolute value of retval */
	mask = (retval >> (sizeof(fix32_t)*CHAR_BIT-1));
	abs_retval = (retval + mask) ^ mask;
	/* So improve its precision by adding some x^4 component to retval */
	retval += fix32_mul(fix32_X4_CORRECTION_COMPONENT, fix32_mul(retval, abs_retval) - retval );
	#endif
	return retval;
}

fix32_t fix32_sin(fix32_t inAngle)
{
	fix32_t tempAngle = inAngle % (fix32_pi << 1);

	if(tempAngle > fix32_pi)
		tempAngle -= (fix32_pi << 1);
	else if(tempAngle < -fix32_pi)
		tempAngle += (fix32_pi << 1);

	#ifndef FIXMATH_NO_CACHE
	fix32_t tempIndex = ((inAngle >> 5) & 0x00000FFF);
	if(_fix32_sin_cache_index[tempIndex] == inAngle)
		return _fix32_sin_cache_value[tempIndex];
	#endif

	fix32_t tempAngleSq = fix32_mul(tempAngle, tempAngle);

	// Most accurate version, accurate to ~2.1%
	fix32_t tempOut = tempAngle;
	tempAngle = fix32_mul(tempAngle, tempAngleSq);
	tempOut -= (tempAngle / 6);
	tempAngle = fix32_mul(tempAngle, tempAngleSq);
	tempOut += (tempAngle / 120);
	tempAngle = fix32_mul(tempAngle, tempAngleSq);
	tempOut -= (tempAngle / 5040);
	tempAngle = fix32_mul(tempAngle, tempAngleSq);
	tempOut += (tempAngle / 362880);
	tempAngle = fix32_mul(tempAngle, tempAngleSq);
	tempOut -= (tempAngle / 39916800);

	#ifndef FIXMATH_NO_CACHE
	_fix32_sin_cache_index[tempIndex] = inAngle;
	_fix32_sin_cache_value[tempIndex] = tempOut;
	#endif

	return tempOut;
}

fix32_t fix32_cos(fix32_t inAngle)
{
	return fix32_sin(inAngle + (fix32_pi >> 1));
}

fix32_t fix32_tan(fix32_t inAngle)
{
	return fix32_sdiv(fix32_sin(inAngle), fix32_cos(inAngle));
}

fix32_t fix32_asin(fix32_t x)
{
	if((x > fix32_one)
		|| (x < -fix32_one))
		return 0;

	fix32_t out;
	out = (fix32_one - fix32_mul(x, x));
	out = fix32_div(x, fix32_sqrt(out));
	out = fix32_atan(out);
	return out;
}

fix32_t fix32_acos(fix32_t x)
{
	return ((fix32_pi >> 1) - fix32_asin(x));
}

fix32_t fix32_atan2(fix32_t inY , fix32_t inX)
{
	fix32_t abs_inY, mask, angle, r, r_3;

	#ifndef FIXMATH_NO_CACHE
	uintptr_t hash = (inX ^ inY);
	hash ^= hash >> 20;
	hash &= 0x0FFF;
	if((_fix32_atan_cache_index[0][hash] == inX) && (_fix32_atan_cache_index[1][hash] == inY))
		return _fix32_atan_cache_value[hash];
	#endif

	/* Absolute inY */
	mask = (inY >> (sizeof(fix32_t)*CHAR_BIT-1));
	abs_inY = (inY + mask) ^ mask;

	// 3rd order Chebyshev polynomial approximation with weight a = 0.189514164974601 and b = 0.970562748477141
	if (inX >= 0)
	{
		r = fix32_div( (inX - abs_inY), (inX + abs_inY));
		r_3 = fix32_mul(fix32_mul(r, r),r);
		angle = fix32_mul(0x0000000030840015LL , r_3) - fix32_mul(0x00000000F876CCE0LL,r) + fix32_PI_DIV_4;
	} else {
		r = fix32_div( (inX + abs_inY), (abs_inY - inX));
		r_3 = fix32_mul(fix32_mul(r, r),r);
		angle = fix32_mul(0x0000000030840015LL , r_3)
			- fix32_mul(0x00000000F876CCE0LL,r)
			+ fix32_THREE_PI_DIV_4;
	}
	if (inY < 0)
	{
		angle = -angle;
	}

	#ifndef FIXMATH_NO_CACHE
	_fix32_atan_cache_index[0][hash] = inX;
	_fix32_atan_cache_index[1][hash] = inY;
	_fix32_atan_cache_value[hash] = angle;
	#endif

	return angle;
}

fix32_t fix32_atan(fix32_t x)
{
	return fix32_atan2(x, fix32_one);
}
