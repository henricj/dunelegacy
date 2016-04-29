#include <fixmath/fix32.h>
#include <stdbool.h>
#include <ctype.h>

static const uint64_t scales[16] = {
    /* 10 decimals is enough for full fix32_t precision */
    1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL, 10000000000ULL, 10000000000ULL, 10000000000ULL, 10000000000ULL, 10000000000ULL, 10000000000ULL
};

static char *itoa_loop(char *buf, uint64_t scale, uint64_t value, bool skip)
{
    while (scale)
    {
        unsigned digit = (value / scale);

        if (!skip || digit || scale == 1)
        {
            skip = false;
            *buf++ = '0' + digit;
            value %= scale;
        }

        scale /= 10;
    }
    return buf;
}

void fix32_to_str(fix32_t value, char *buf, int decimals)
{
    uint64_t uvalue = (value >= 0) ? value : -value;
    if (value < 0)
        *buf++ = '-';

    /* Separate the integer and decimal parts of the value */
    uint64_t intpart = uvalue >> 32;
    uint64_t fracpart = uvalue & 0xFFFFFFFF;
    uint64_t scale = scales[decimals & 0xF];
    fracpart = fix32_mul(fracpart, scale);

    if (fracpart >= scale)
    {
        /* Handle carry from decimal part */
        intpart++;
        fracpart -= scale;
    }

    /* Format integer part */
    buf = itoa_loop(buf, 1000000000, intpart, true);

    /* Format decimal part (if any) */
    if (scale != 1)
    {
        *buf++ = '.';
        buf = itoa_loop(buf, scale / 10, fracpart, false);
    }

    *buf = '\0';
}

fix32_t fix32_from_str(const char *buf)
{
    while (isspace(*buf))
        buf++;

    /* Decode the sign */
    bool negative = (*buf == '-');
    if (*buf == '+' || *buf == '-')
        buf++;

    /* Decode the integer part */
    uint64_t intpart = 0;
    int count = 0;
    while (isdigit(*buf))
    {
        intpart *= 10;
        intpart += *buf++ - '0';
        count++;
    }

    if (count == 0 || count > 10
        || intpart > 2147483648LL || (!negative && intpart > 2147483647LL))
        return fix32_overflow;

    fix32_t value = intpart << 32;

    /* Decode the decimal part */
    if (*buf == '.' || *buf == ',')
    {
        buf++;

        uint64_t fracpart = 0;
        uint64_t scale = 1;
        while (isdigit(*buf) && scale < 10000000000LL)
        {
            scale *= 10;
            fracpart *= 10;
            fracpart += *buf++ - '0';
        }

        value += fix32_div(fracpart, scale);
    }

    /* Verify that there is no garbage left over */
    while (*buf != '\0')
    {
        if (!isdigit(*buf) && !isspace(*buf))
            return fix32_overflow;

        buf++;
    }

    return negative ? -value : value;
}

