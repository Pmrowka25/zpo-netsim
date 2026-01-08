// Helper objects and global functions

#pragma once

#include "types.hpp"
#include <functional>
#include <random>

namespace NetSim {
/**
 * @brief Generates pseudo-random numbers in range [0, 1), 10 bits of randomness
 */
double default_probability_generator();

/**
 * @brief Declaringglobal object, which is a function
 */
extern ProbabilityGenerator probability_generator; // extern means that this function is defined somethere else

} // namespace NetSim
