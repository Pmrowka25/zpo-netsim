#pragma once

#include "factory.hpp"
#include <functional>
#include <set>

namespace NetSim {

void simulate(Factory& f, TimeOffset d, std::function<void(Factory&, Time)> rf);

class IntervalReportNotifier {
public:
    explicit IntervalReportNotifier(TimeOffset to) : to_(to) {}
    bool should_generate_report(Time t) const { return (t - 1) % to_ == 0; }
private:
    TimeOffset to_;
};

class SpecificTurnsReportNotifier {
public:
    explicit SpecificTurnsReportNotifier(std::set<Time> turns) : turns_(std::move(turns)) {}
    bool should_generate_report(Time t) const { return turns_.find(t) != turns_.end(); }
private:
    std::set<Time> turns_;
};

} // namespace NetSim
