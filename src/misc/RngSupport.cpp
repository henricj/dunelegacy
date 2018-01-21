#include "misc/RngSupport.h"


#include <cassert>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <random>

using namespace std;

static vector<unsigned int> CreateRootSeedVector()
{
    random_device rd;

    std::vector<decltype(rd)::result_type> seed_data;

    const int reserve_size = Nyq::nyq_generator_state_size + 3;

    seed_data.reserve(reserve_size);

    generate_n(std::back_inserter(seed_data), Nyq::nyq_generator_state_size, ref(rd));

    // Protect against a broken random_device
    const auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    seed_data.push_back(static_cast<unsigned int>(timestamp) & 0xffffffff);
    seed_data.push_back(static_cast<unsigned int>(timestamp >> 32));

    static atomic<int> counter;

    const auto x = counter.fetch_add(1, memory_order_relaxed);

    seed_data.push_back(x);

    assert(seed_data.size() == reserve_size);

    return seed_data;
}

template <class RNG = Nyq::nyq_generator>
static RNG CreateRootGenerator()
{
    auto seed_data = CreateRootSeedVector();

    std::seed_seq seed(seed_data.begin(), seed_data.end());

    return RNG{seed};
}

#ifdef _MSC_VER
#if _MSC_VER < 1900
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL thread_local
#endif
#else
#define THREAD_LOCAL __thread
#endif

template <class RNG = Nyq::nyq_generator>
static RNG& GetRootGenerator()
{
    static THREAD_LOCAL RNG generator{ CreateRootGenerator<RNG>() };

    return generator;
}

namespace Nyq
{
namespace RngSupport
{
std::vector<unsigned int> CreateSeedVector(std::vector<unsigned int>::size_type size)
{
    if (size < 1)
       size = 1;

    vector<unsigned int> seed;

    seed.reserve(size);

    auto& rng = GetRootGenerator();

    std::uniform_int_distribution<unsigned> uniform;

    std::generate_n(std::back_inserter(seed), size, [&] { return uniform(rng); });

    return seed;
}
} // namespace RngSupport
} // namespace Nyq

extern "C"
void RandomFillUniformFloat(float* p, int count, float low, float high)
{
    if (count < 1)
        return;

    auto& generator = GetRootGenerator();

    std::uniform_real_distribution<float> uniform{ low, high };

    std::generate_n(p, count, [&] { return uniform(generator); });
}

extern "C"
void RandomFillNormalFloat(float* p, int count, float mean, float sigma)
{
    if (count < 1)
        return;

    Nyq::nyq_generator& generator = GetRootGenerator();

    std::normal_distribution<float> normal{ mean, sigma };

    std::generate_n(p, count, [&] { return normal(generator); });
}

extern "C"
int RandomFillClampedNormalFloat(float* p, int count, float mean, float sigma, float low, float high)
{
    if (count < 1)
        return 1;

    Nyq::nyq_generator& generator = GetRootGenerator();

    std::normal_distribution<float> normal{ mean, sigma };

    while (count--)
    {
       auto retry = 10;

        for (;;)
        {
           const auto x = normal(generator);

            if (x <= high && x >= low)
            {
                *p++ = x;
                break;
            }

            if (--retry <= 0)
                return 0;
        }
    }

    return 1;
}

extern "C"
float RandomUniformFloat(float low, float high)
{
    std::uniform_real_distribution<float> uniform{ low, high };

   return uniform(GetRootGenerator());
}

extern "C"
double RandomUniformDouble(double low, double high)
{
    std::uniform_real_distribution<> uniform{ low, high };

   return uniform(GetRootGenerator());
}

extern "C"
int RandomUniformInt(int lowInclusive, int highInclusive)
{
    std::uniform_int_distribution<> uniform{ lowInclusive, highInclusive };

    return uniform(GetRootGenerator());
}

extern "C"
long RandomUniformLong(long lowInclusive, long highInclusive)
{
    std::uniform_int_distribution<long> uniform{ lowInclusive, highInclusive };

   return uniform(GetRootGenerator());
}

