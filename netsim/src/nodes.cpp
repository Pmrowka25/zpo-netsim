#include "nodes.hpp"

namespace NetSim {

// RECEIVER PREFERENCES

ReceiverPreferences::ReceiverPreferences(ProbabilityGenerator pg) : pg_(pg) {}

void ReceiverPreferences::add_receiver(IPackageReceiver *receiver) {
    preferences_[receiver] = 1.0; // add with default probability and then scale

    double new_prob = 1.0 / preferences_.size();
    for (auto &pair : preferences_) { // reference to the map of preferences
        pair.second = new_prob;
    }
}

void ReceiverPreferences::remove_receiver(IPackageReceiver *receiver) {
    preferences_.erase(receiver);

    if (preferences_.empty())
        return; // return if there are no receivers left

    double new_prob = 1.0 / preferences_.size();
    for (auto &pair : preferences_) { // reference to the map of preferences
        pair.second = new_prob;
    }
}

IPackageReceiver *ReceiverPreferences::choose_receiver() {
    double p = pg_(); // Picks a number from [0,1]
    double distribution = 0.0;

    // Distribution
    // Iterating till distribution exceeds the probability
    for (const auto &pair :
         preferences_) {             // reference to the map of preferences
        distribution += pair.second; // Accumulating probability
        if (p <= distribution)
            return pair.first;
    }

    // Numerical error safety net
    if (!preferences_.empty())
        return preferences_.rbegin()->first;

    return nullptr;
}

const ReceiverPreferences::preferences_t &
ReceiverPreferences::get_preferences() const {
    return preferences_;
}

ReceiverPreferences::const_iterator ReceiverPreferences::begin() const {
    return preferences_.begin();
}
ReceiverPreferences::const_iterator ReceiverPreferences::end() const {
    return preferences_.end();
}
ReceiverPreferences::const_iterator ReceiverPreferences::cbegin() const {
    return preferences_.cbegin();
}
ReceiverPreferences::const_iterator ReceiverPreferences::cend() const {
    return preferences_.cend();
}

// PACKAGE SENDER

void PackageSender::send_package() {
    if (buffer_) {
        IPackageReceiver *receiver =
            receiver_preferences_
                .choose_receiver(); // calling choose_receiver on the instance
                                    // of RecerverPreferences
        if (receiver) {             // When receiver is succesfully picked
            receiver->receive_package(
                std::move(*buffer_)); // call receive_package method to collect
                                      // what's in the buffer
            buffer_.reset();          // Empty the buffer
        }
    }
}

const ReceiverPreferences &PackageSender::get_receiver_preferences() const {
    return receiver_preferences_;
}

ReceiverPreferences &PackageSender::get_receiver_preferences() {
    return receiver_preferences_;
}

void PackageSender::push_package(Package &&package) {
    buffer_.emplace(std::move(package));
}

// No implementation of IPackageReceiver since it's an abstract class - purely
// virtual, an interface

// RAMP

Ramp::Ramp(ElementID id, TimeOffset di) : id_(id), delivery_interval_(di) {};

void Ramp::deliver_goods(Time t) {
    if ((t - 1) % delivery_interval_ ==
        0) { // starting from time t=1, so it ALWAYS generates a package at the
             // start round
        Package p;
        push_package(std::move(p));
    }
}

ElementID Ramp::get_id() const { return id_; }

TimeOffset Ramp::get_delivery_interval() const { return delivery_interval_; }

// WORKER

Worker::Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q)
    : id_(id), processing_duration_(pd), q_(std::move(q)) {
      }; // q is a smart pointer, it cannot be coppied, must be moved

void Worker::receive_package(Package &&p) {
    q_->push(std::move(p)); // Insert incoming package to the queue, not
                            // disturbing current work
}

void Worker::do_work(Time t) {
    if (!processing_buffer_ &&
        !q_->empty()) { // if currently not working and buffer not empty
        processing_buffer_.emplace(q_->pop()); // take package from input queue
        package_processing_start_time_ = t;
    }

    if (processing_buffer_) { // if currently working
        if (t - package_processing_start_time_ >=
            processing_duration_ - 1) // if all processing has been done, sends
                                      // package in the next round
        {
            push_package(std::move(*processing_buffer_));
            processing_buffer_.reset();

            // it cannot take another package till the next round
        }
    }
}

ReceiverType Worker::get_receiver_type() const { return ReceiverType::WORKER; }

ElementID Worker::get_id() const { return id_; }

TimeOffset Worker::get_processing_duration() const {
    return processing_duration_;
}

Time Worker::get_product_processing_start_time() const {
    return package_processing_start_time_;
}

IPackageQueue* Worker::get_queue() const { return q_.get(); }

Worker::const_iterator Worker::begin() const { return q_->begin(); }
Worker::const_iterator Worker::end() const { return q_->end(); }
Worker::const_iterator Worker::cbegin() const { return q_->cbegin(); }
Worker::const_iterator Worker::cend() const { return q_->cend(); }

// STOREHOUSE

Storehouse::Storehouse(
    ElementID id,
    std::unique_ptr<IPackageStockpile> d) // don't define default argument twice
    : id_(id), d_(std::move(d)) {
} // d_ is a pointer, so then d_->push must be done instead of d.push

ReceiverType Storehouse::get_receiver_type() const {
    return ReceiverType::STOREHOUSE;
}

void Storehouse::receive_package(Package &&p) { d_->push(std::move(p)); }

ElementID Storehouse::get_id() const { return id_; }

Storehouse::const_iterator Storehouse::begin() const { return d_->begin(); }
Storehouse::const_iterator Storehouse::end() const { return d_->end(); }
Storehouse::const_iterator Storehouse::cbegin() const { return d_->cbegin(); }
Storehouse::const_iterator Storehouse::cend() const { return d_->cend(); }

} // namespace NetSim