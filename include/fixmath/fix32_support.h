#pragma once

namespace fix32_support
{
    namespace implementation
    {
#pragma warning(push)
        // The unused branches in the template functions have integer overflows.
#pragma warning(disable:4307)

        template<uint64_t I, uint64_t F, uint64_t Scale>
        static constexpr uint64_t fp32_fraction_parser()
        {
            return ((I) << 32) | (((F) << 32) / Scale);
        }

        template<uint64_t I, uint64_t F, uint64_t Scale, char First, char ...Chars>
        static constexpr uint64_t fp32_fraction_parser()
        {
            return fp32_fraction_parser<I, 10 * F + (First - '0'), 10 * Scale, Chars...>();
        }

        template<uint64_t I>
        static constexpr uint64_t fp32_parser()
        {
            return (I) << 32;
        }

        template<uint64_t I, char First, char...Chars>
        static constexpr uint64_t fp32_parser()
        {
            static_assert(First == '.' || (First >= '0' && First <= '9'));

            if (First == '.')
                return fp32_fraction_parser<I, 0ull, 1, Chars...>();

            return fp32_parser<10 * I + (static_cast<int>(First) - '0'), Chars...>();
        }
#pragma warning(pop)
    }
    template<char...Chars>
    constexpr uint64_t operator"" _fix32()
    {
        return implementation::fp32_parser<0ull, Chars...>();
    }
}

