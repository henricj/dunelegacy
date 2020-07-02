#ifndef RNGSUPPORT_H
#define RNGSUPPORT_H

#include <vector>
#include <random>
#include "random_xoshiro256starstar.h"
#include "lemire_uniform_uint32_distribution.h"

namespace Dune
{
typedef std::seed_seq nyq_seed_seq;
//typedef std::uniform_real_distribution<float> nyq_uniform_float_distribution;
//typedef std::uniform_real_distribution<double> nyq_uniform_double_distribution;
//typedef std::normal_distribution<float> nyq_normal_float_distribution;
typedef std::uniform_int_distribution<int> nyq_uniform_int_distribution;
typedef std::uniform_int_distribution<long> nyq_uniform_long_distribution;

namespace RngSupport
{
typedef ExtraGenerators::xoshiro256starstar nyq_generator;

nyq_generator CreateGenerator();
} // namespace RngSupport


class NyqEngine : public RngSupport::nyq_generator
{
public:
   explicit NyqEngine() : RngSupport::nyq_generator(RngSupport::CreateGenerator())
   { }
};

} // namespace Dune


#endif // RNGSUPPORT_H
