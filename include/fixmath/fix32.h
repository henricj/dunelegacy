#ifndef __libfixmath_fix32_h__
#define __libfixmath_fix32_h__

#ifdef __cplusplus
extern "C"
{
#endif

/* These options may let the optimizer to remove some calls to the functions.
 * Refer to http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
 */
#ifndef FIXMATH_FUNC_ATTRS
# ifdef __GNUC__
#   if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6)
#     define FIXMATH_FUNC_ATTRS __attribute__((leaf, nothrow, const))
#   else
#     define FIXMATH_FUNC_ATTRS __attribute__((nothrow, const))
#   endif
# else
#   define FIXMATH_FUNC_ATTRS
# endif
#endif

#include <stdint.h>

typedef int64_t fix32_t;

static const fix32_t fix32_FOUR_DIV_PI  = 0x0000000145F306DCULL;             /*!< Fix32 value of 4/PI */
static const fix32_t fix32__FOUR_DIV_PI2 = 0xFFFFFFFF983F4277ULL;            /*!< Fix32 value of -4/PI² */
static const fix32_t fix32_X4_CORRECTION_COMPONENT = 0x000000003999999AULL;  /*!< Fix32 value of 0.225 */
static const fix32_t fix32_PI_DIV_4 = 0x00000000C90FDAA2ULL;                 /*!< Fix32 value of PI/4 */
static const fix32_t fix32_THREE_PI_DIV_4 = 0x000000025B2F8FE6ULL;           /*!< Fix32 value of 3PI/4 */

static const fix32_t fix32_maximum  = 0x7FFFFFFFFFFFFFFFULL; /*!< the maximum value of fix32_t */
static const fix32_t fix32_minimum  = 0x8000000000000000ULL; /*!< the minimum value of fix32_t */
static const fix32_t fix32_overflow = 0x8000000000000000ULL; /*!< the value used to indicate overflows when FIXMATH_NO_OVERFLOW is not specified */

static const fix32_t fix32_pi  = 0x00000003243F6A89ULL;     /*!< fix32_t value of pi */
static const fix32_t fix32_e   = 0x00000002B7E15163ULL;     /*!< fix32_t value of e */
static const fix32_t fix32_one = 0x0000000100000000ULL;     /*!< fix32_t value of 1 */

/* Conversion functions between fix32_t and float/integer.
 * These are inlined to allow compiler to optimize away constant numbers
 */
static inline fix32_t fix32_from_int(int a)     { return a * fix32_one; }
static inline float   fix32_to_float(fix32_t a) { return (float)a / fix32_one; }
static inline double  fix32_to_dbl(fix32_t a)   { return (double)a / fix32_one; }
#ifdef __libfixmath_fix16_h__
static inline fix32_t fix32_from_fix16(fix16_t a)     { if(a == fix16_overflow) { return fix32_overflow; } else { return ((fix32_t) a) << 16; } }
static inline fix16_t fix16_from_fix32(fix32_t a)     { fix32_t tmp = a >> 16; if(tmp > fix16_maximum || tmp < fix16_minimum) { tmp = fix16_overflow; } return (fix16_t) tmp; }
#endif

static inline int fix32_to_int(fix32_t a)
{
#ifdef FIXMATH_NO_ROUNDING
    return (a >> 32);
#else
    if (a >= 0)
        return (int) ((a + (fix32_one >> 1)) / fix32_one);
    return (int) ((a - (fix32_one >> 1)) / fix32_one);
#endif
}

static inline fix32_t fix32_from_float(float a)
{
    float temp = a * fix32_one;
#ifndef FIXMATH_NO_ROUNDING
    temp += (temp >= 0) ? 0.5f : -0.5f;
#endif
    return (fix32_t)temp;
}

static inline fix32_t fix32_from_dbl(double a)
{
    double temp = a * fix32_one;
#ifndef FIXMATH_NO_ROUNDING
    temp += (temp >= 0) ? 0.5 : -0.5;
#endif
    return (fix32_t)temp;
}

/* Macro for defining fix32_t constant values.
   The functions above can't be used from e.g. global variable initializers,
   and their names are quite long also. This macro is useful for constants
   springled alongside code, e.g. F32(1.234).

   Note that the argument is evaluated multiple times, and also otherwise
   you should only use this for constant values. For runtime-conversions,
   use the functions above.
*/
#define F32(x) ((fix32_t)(((x) >= 0) ? ((x) * 4294967296.0 + 0.5) : ((x) * 4294967296.0 - 0.5)))

static inline fix32_t fix32_abs(fix32_t x)
    { return (x < 0 ? -x : x); }
static inline fix32_t fix32_floor(fix32_t x)
    { return (x & 0xFFFFFFFF00000000ULL); }
static inline fix32_t fix32_ceil(fix32_t x)
    { return (x & 0xFFFFFFFF00000000ULL) + ((x & 0x00000000FFFFFFFFULL) ? fix32_one : 0); }
static inline fix32_t fix32_min(fix32_t x, fix32_t y)
    { return (x < y ? x : y); }
static inline fix32_t fix32_max(fix32_t x, fix32_t y)
    { return (x > y ? x : y); }
static inline fix32_t fix32_clamp(fix32_t x, fix32_t lo, fix32_t hi)
    { return fix32_min(fix32_max(x, lo), hi); }

/* Subtraction and addition with (optional) overflow detection. */
#ifdef FIXMATH_NO_OVERFLOW

static inline fix32_t fix32_add(fix32_t inArg0, fix32_t inArg1) { return (inArg0 + inArg1); }
static inline fix32_t fix32_sub(fix32_t inArg0, fix32_t inArg1) { return (inArg0 - inArg1); }

#else

extern fix32_t fix32_add(fix32_t a, fix32_t b) FIXMATH_FUNC_ATTRS;
extern fix32_t fix32_sub(fix32_t a, fix32_t b) FIXMATH_FUNC_ATTRS;

/* Saturating arithmetic */
extern fix32_t fix32_sadd(fix32_t a, fix32_t b) FIXMATH_FUNC_ATTRS;
extern fix32_t fix32_ssub(fix32_t a, fix32_t b) FIXMATH_FUNC_ATTRS;

#endif

/*! Multiplies the two given fix32_t's and returns the result.
*/
extern fix32_t fix32_mul(fix32_t inArg0, fix32_t inArg1) FIXMATH_FUNC_ATTRS;

/*! Divides the first given fix32_t by the second and returns the result.
*/
extern fix32_t fix32_div(fix32_t inArg0, fix32_t inArg1) FIXMATH_FUNC_ATTRS;

#ifndef FIXMATH_NO_OVERFLOW
/*! Performs a saturated multiplication (overflow-protected) of the two given fix32_t's and returns the result.
*/
extern fix32_t fix32_smul(fix32_t inArg0, fix32_t inArg1) FIXMATH_FUNC_ATTRS;

/*! Performs a saturated division (overflow-protected) of the first fix32_t by the second and returns the result.
*/
extern fix32_t fix32_sdiv(fix32_t inArg0, fix32_t inArg1) FIXMATH_FUNC_ATTRS;
#endif

/*! Divides the first given fix32_t by the second and returns the result.
*/
extern fix32_t fix32_mod(fix32_t x, fix32_t y) FIXMATH_FUNC_ATTRS;













/*! Returns the sine of the given fix32_t.
*/
extern fix32_t fix32_sin_parabola(fix32_t inAngle) FIXMATH_FUNC_ATTRS;

/*! Returns the sine of the given fix32_t.
*/
extern fix32_t fix32_sin(fix32_t inAngle) FIXMATH_FUNC_ATTRS;

/*! Returns the cosine of the given fix32_t.
*/
extern fix32_t fix32_cos(fix32_t inAngle) FIXMATH_FUNC_ATTRS;

/*! Returns the tangent of the given fix32_t.
*/
extern fix32_t fix32_tan(fix32_t inAngle) FIXMATH_FUNC_ATTRS;

/*! Returns the arcsine of the given fix32_t.
*/
extern fix32_t fix32_asin(fix32_t inValue) FIXMATH_FUNC_ATTRS;

/*! Returns the arccosine of the given fix32_t.
*/
extern fix32_t fix32_acos(fix32_t inValue) FIXMATH_FUNC_ATTRS;

/*! Returns the arctangent of the given fix32_t.
*/
extern fix32_t fix32_atan(fix32_t inValue) FIXMATH_FUNC_ATTRS;

/*! Returns the arctangent of inY/inX.
*/
extern fix32_t fix32_atan2(fix32_t inY, fix32_t inX) FIXMATH_FUNC_ATTRS;

static const fix32_t fix32_rad_to_deg_mult = 0x000000394BB834C8LL;
static inline fix32_t fix32_rad_to_deg(fix32_t radians)
    { return fix32_mul(radians, fix32_rad_to_deg_mult); }

static const fix32_t fix32_deg_to_rad_mult = 0x000000000477D1A8LL;
static inline fix32_t fix32_deg_to_rad(fix32_t degrees)
    { return fix32_mul(degrees, fix32_deg_to_rad_mult); }



/*! Returns the square root of the given fix32_t.
*/
extern fix32_t fix32_sqrt(fix32_t inValue) FIXMATH_FUNC_ATTRS;

/*! Returns the square of the given fix32_t.
*/
static inline fix32_t fix32_sq(fix32_t x)
    { return fix32_mul(x, x); }

/*! Returns the exponent (e^) of the given fix32_t.
*/
extern fix32_t fix32_exp(fix32_t inValue) FIXMATH_FUNC_ATTRS;

/*! Returns the natural logarithm of the given fix32_t.
 */
extern fix32_t fix32_log(fix32_t inValue) FIXMATH_FUNC_ATTRS;

/*! Returns the base 2 logarithm of the given fix32_t.
 */
extern fix32_t fix32_log2(fix32_t x) FIXMATH_FUNC_ATTRS;

/*! Returns the saturated base 2 logarithm of the given fix32_t.
 */
extern fix32_t fix32_slog2(fix32_t x) FIXMATH_FUNC_ATTRS;

/*! Convert fix32_t value to a string.
 * Required buffer length for largest values is 24 bytes.
 */
extern void fix32_to_str(fix32_t value, char *buf, int decimals);

/*! Convert string to a fix32_t value
 * Ignores spaces at beginning and end. Returns fix32_overflow if
 * value is too large or there were garbage characters.
 */
extern fix32_t fix32_from_str(const char *buf);

/** Helper macro for F32C. Replace token with its number of characters/digits. */
#define FIXMATH_TOKLEN(token) ( sizeof( #token ) - 1 )

/** Helper macro for F32C. Handles pow(10, n) for n from 0 to 16. */
#define FIXMATH64_CONSTANT_POW10(times) ( \
  (times == 0) ? 1ULL \
        : (times == 1) ? 10ULL \
            : (times == 2) ? 100ULL \
                : (times == 3) ? 1000ULL \
                    : (times == 4) ? 10000ULL \
                        : (times == 5) ? 100000ULL \
                            : (times == 6) ? 1000000ULL \
                                : (times == 7) ? 10000000ULL \
                                    : (times == 8) ? 100000000ULL \
                                        : (times == 9) ? 1000000000ULL \
                                            : (times == 10) ? 10000000000ULL \
                                                : (times == 11) ? 100000000000ULL \
                                                    : (times == 12) ? 1000000000000ULL \
                                                        : (times == 13) ? 10000000000000ULL \
                                                            : (times == 14) ? 100000000000000ULL \
                                                                : (times == 15) ? 1000000000000000ULL \
                                                                    : 10000000000000000ULL \
)


/** Helper macro for F32C.
 *
 * @note We do not use fix32_one instead of 4294967296ULL, because the
 *       "use of a const variable in a constant expression is nonstandard in C".
 */
#define FIXMATH64_CONVERT_MANTISSA(m) \
( (unsigned) \
    ( \
        ( \
            ( \
                (uint64_t)( ( ( 1 ## m ## ULL ) - FIXMATH64_CONSTANT_POW10(FIXMATH_TOKLEN(m)) ) * FIXMATH64_CONSTANT_POW10(9 - FIXMATH_TOKLEN(m)) ) \
                * 4294967296ULL \
            ) \
        ) \
        / \
        1000000000LL \
    ) \
)


#define FIXMATH64_COMBINE_I_M(i, m) \
( \
    ( \
        (    i ) \
        << 32 \
    ) \
    | \
    ( \
        FIXMATH64_CONVERT_MANTISSA(m) \
        & 0xFFFFFFFF \
    ) \
)


/** Create int16_t (Q32.32) constant from separate integer and mantissa part.
 *
 * Only tested on 32-bit ARM Cortex-M0 / x86 Intel.
 *
 * This macro is needed when compiling with options like "--fpu=none",
 * which forbid all and every use of float and related types and
 * would thus make it impossible to have fix32_t constants.
 *
 * Just replace uses of F32() with F32C() like this:
 *   F32(123.1234) becomes F32C(123,1234)
 *
 * @warning Specification of any value outside the mentioned intervals
 *          WILL result in undefined behavior!
 *
 * @note Regardless of the specified minimum and maximum values for i and m below,
 *       the total value of the number represented by i and m MUST be in the interval
 *       ]-2147483648.000000000:2147483647.999999999[ else usage with this macro will yield undefined behavior.
 *
 * @param i Signed integer constant with a value in the interval ]-2147483648:2147483647[.
 * @param m Positive integer constant in the interval ]0:999999999[ (fractional part/mantissa).
 */
#define F32C(i, m) \
( (fix32_t) \
    ( \
      (( #i[0] ) == '-') \
        ? -FIXMATH64_COMBINE_I_M(( ( (i ## LL ) * -1LL) ), m) \
        : FIXMATH64_COMBINE_I_M((uint64_t)( ( i ## LL ) ) , m) \
    ) \
)

#ifdef __cplusplus
}
#endif

#endif
