#include "misc/RngSupport.h"

#include <memory>

#include <cassert>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <random>

using namespace std;
#if defined(MSC_VER) && MSC_VER < 1900
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL thread_local
#endif

namespace Dune {
namespace RngSupport
{
const int nyq_generator_seed_words = nyq_generator::seed_words;

vector<unsigned int> CreateRootSeedVector()
{
   random_device rd;

   std::vector<decltype(rd)::result_type> seed_data;

   const int reserve_size = nyq_generator_seed_words + 3;

   seed_data.reserve(reserve_size);

   generate_n(std::back_inserter(seed_data), nyq_generator_seed_words, ref(rd));

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

nyq_generator CreateRootGenerator()
{
   auto seed_data = CreateRootSeedVector();

   std::seed_seq seed(seed_data.begin(), seed_data.end());

   return nyq_generator{ seed };
}

namespace {
   nyq_generator& GetGenerator()
   {
      THREAD_LOCAL nyq_generator generator = CreateRootGenerator();

      return generator;
   }
}

nyq_generator CreateGenerator()
{
   auto& generator = GetGenerator();

   const auto ret = generator;

   generator.jump();

   return ret;
}

} // namespace RngSupport
} // namespace Dune

