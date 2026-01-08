#pragma once

#include "package.hpp"
#include <deque>
#include <list>

namespace NetSim {
/**
 * @brief Enumeration type defining a quque type
 */
enum class PackageQueueType { FIFO, LIFO };

/**
 * @brief Interface for any product container
 * Defines basic operations: add, check state that all container types use:
 * WorkerQueue, Storehouse Implementation of storing a Package is not included
 * here becase worker may store it in std::queue and Storehouse just in
 * std::vector
 */
class IPackageStockpile {

public:
  // Defining iterator alias based on list
  using const_iterator = std::list<Package>::const_iterator;

  // Constructor is not needed for the interface as it's purely virual and never
  // initialized

  /**
   * @brief Always add destructor to polimorphic interfaces
   * When derived class points to the base class and base class is destroyed
   * it makes sure that derived class will be cleaned as well, prevent memory
   * leak
   */
  virtual ~IPackageStockpile() {}

  /**
   * @brief Add a product to the container
   */
  virtual void push(Package &&package) = 0;

  /**
   * @brief Check if container is empty
   */
  virtual bool empty() const = 0;

  /**
   * @brief Return the amount of elements in a container (size_t is preferred
   * type for sizes)
   */
  virtual size_t size() const = 0;

  // Iterator methods (for reporting)

  virtual const_iterator begin() const = 0;
  virtual const_iterator end() const = 0;
  virtual const_iterator cbegin() const = 0;
  virtual const_iterator cend() const = 0;
};

/**
 * @brief Interface for any product queue
 * Inherits from IPackageStockpile - every Queue is a Stockpile
 * Adds method for retrieving a Package from the container according to a
 * defined strategy (FIFO, LIFO)
 */
class IPackageQueue : public IPackageStockpile {
public:
  /**
   * @brief Rethrievs an element from a queue
   * Order depends on implmentation (FIFO/LIFO), implements IF statement (not
   * ideally correct with OCP) It's better however to violate OCP than DRY
   * principle
   * @return Package (product)
   */
  virtual Package pop() = 0;

  /**
   * @brief Returns Queue type
   * @return Enumerate type PackageQueueType
   */
  virtual PackageQueueType get_queue_type() const = 0;

  virtual ~IPackageQueue() = default;
};

/**
 * @brief Specifc Queue implementation, handles both FIFO and LIFO
 */
class PackageQueue : public IPackageQueue {
public:
  /**
   * @brief Constructor
   * @param type PackageQueueType (FIFO or LIFO)
   */
  explicit PackageQueue(
      PackageQueueType
          type); // explicit block type conversion, so that constructor allows
                 // only PackageQueueType as an argument

  void push(Package &&package) override; // Package&& so content of the package
                                         // is fully moved, not just coppied
  bool empty() const override;
  size_t size() const override;
  Package pop() override;
  PackageQueueType get_queue_type() const override;

  // Iterator methods implementation

  const_iterator begin() const override;
  const_iterator end() const override;
  const_iterator cbegin() const override;
  const_iterator cend() const override;

  ~PackageQueue() override =
      default; // Name is different form IPackageQueue but

private:
  PackageQueueType queue_type_;
  std::list<Package> container_; // std::list avoids iterator invalidation
};

} // namespace NetSim
