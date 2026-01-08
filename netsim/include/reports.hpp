#pragma once

#include "factory.hpp"
#include <ostream>

namespace NetSim {

void generate_structure_report(const Factory& factory, std::ostream& os);

void generate_simulation_turn_report(const Factory& factory, std::ostream& os, Time t);

} // namespace NetSim
