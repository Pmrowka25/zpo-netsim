#include "package.hpp"

namespace NetSim {

std::set<ElementID> Package::assigned_ids_ = {};
std::set<ElementID> Package::freed_ids_ = {};

Package::Package() {
  if (!freed_ids_.empty()) {
    id_ = *freed_ids_.begin();
    freed_ids_.erase(freed_ids_.begin());
  } else {
    id_ = (assigned_ids_.empty())
              ? 1
              : (*assigned_ids_.rbegin() +
                 1); // 'rbegin' returns the highest number, 'end' would point
                     // behind the highest number
  }
  assigned_ids_.insert(id_);
}

Package::Package(ElementID id) : id_(id) {
  assigned_ids_.insert(id_); // id_ not id as a form of reassurence that it was
                             // correctly assigned in the initialization list
  freed_ids_.erase(id);
}

Package::Package(Package &&other) noexcept : id_(other.id_) {
  other.id_ = -1; // to prevent other's destructor to assing its original ID to
                  // freed_ids_
}

Package &Package::operator=(Package &&other) noexcept {
  if (this != &other) {
    if (id_ != -1) {
      assigned_ids_.erase(id_);
      freed_ids_.insert(id_);
    }

    id_ = other.id_;
    other.id_ = -1;
  }

  return *this;
}

ElementID Package::get_id() const { return id_; }

Package::~Package() {
  if (id_ != -1) {
    assigned_ids_.erase(id_);
    freed_ids_.insert(id_);
  }
}

} // namespace NetSim
