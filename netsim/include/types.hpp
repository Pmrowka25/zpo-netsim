// aliases

#pragma once

#include <functional>

namespace NetSim {
/**
 * @brief Alias for Nodes and Products ID type
 */
using ElementID = int;

/**
 * @brief Alias for probability (0.0 - 1.0)
 * wrapper for anything that's a funciton - takes no arguments and return
 * double
 */
using ProbabilityGenerator = std::function<double()>;

/**
 * @brief Alias for amount of rounds
 */
using Time = int;

/**
 * @brief Alias for the time offset
 */
using TimeOffset = int;

/**
 * @brief
 */

} // namespace NetSim
