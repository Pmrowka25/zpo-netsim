// Helper objects and global functions

#include "helpers.hpp"

#include <cstdlib>
#include <random>

namespace NetSim {
// Creating static generator once
std::random_device rd;
std::mt19937 rng(rd());

double default_probability_generator() {
  return std::generate_canonical<double, 10>(rng);
}

// Initializing global variable being a function
ProbabilityGenerator probability_generator = default_probability_generator;
} // namespace NetSim