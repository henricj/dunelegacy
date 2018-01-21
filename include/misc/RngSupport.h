#ifndef RNGSUPPORT_H
#define RNGSUPPORT_H

#ifdef __cplusplus

#include <vector>
#include <random>

#include "random_xorshift1024star.h"

namespace Nyq
{
typedef ExtraGenerators::xorshift1024star nyq_generator;
const int nyq_generator_state_size = sizeof(nyq_generator) / sizeof(unsigned);
typedef std::seed_seq nyq_seed_seq;

namespace RngSupport
{
std::vector<unsigned int> CreateSeedVector(std::vector<unsigned int>::size_type size);
} // namespace Rng

template <class RNG = nyq_generator>
static RNG CreateGenerator(int size = 32)
{
    auto seed_data = RngSupport::CreateSeedVector(size);

    nyq_seed_seq seq(seed_data.begin(), seed_data.end());

    return RNG{seq};
}

template <class RNG = nyq_generator>
static void ReseedGenerator(RNG& generator, int size = 32)
{
    auto seed_data = RngSupport::CreateSeedVector(size);

    nyq_seed_seq seq(seed_data.begin(), seed_data.end());

    generator.seed(seq);
}

template <class RNG = nyq_generator>
class NyqEngine : public RNG
{
public:
   explicit NyqEngine(int size = 32) : RNG(CreateGenerator(size))
   { }
};

} // namespace Nyq

extern "C" {
#endif // __cplusplus

    void RandomFillUniformFloat(float* p, int count, float low, float high);
    void RandomFillNormalFloat(float* p, int count, float mean, float sigma);
    int RandomFillClampedNormalFloat(float* p, int count, float mean, float sigma, float low, float high);
    float RandomUniformFloat(float low, float high);
    double RandomUniformDouble(double low, double high);

    int RandomUniformInt(int lowInclusive, int highInclusive);
    long RandomUniformLong(long lowInclusive, long highInclusive);

#ifdef __cplusplus
} //   extern "C"
#endif

#endif // RNGSUPPORT_H
