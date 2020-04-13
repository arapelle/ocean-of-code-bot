#include "random.hpp"

namespace priv
{
std::mt19937_64& rand_int_engine()
{
    static std::mt19937_64 rnd_engine(std::random_device{}());
    return rnd_engine;
}
}
