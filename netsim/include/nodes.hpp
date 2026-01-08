#pragma once

#include "config.hpp"
#include "helpers.hpp"
#include "package.hpp"
#include "storage_types.hpp"
#include "types.hpp"

#include <map>
#include <memory>
#include <optional> // for buffer

namespace NetSim {

enum class ReceiverType { WORKER, STOREHOUSE };

// Forward declaration (ReceiverPreferences uses it)
class IPackageReceiver;

/**
 * @brief Helper class, concrete
 * Stores map (receiver -> probability) and let's pick a receiver
 */
class ReceiverPreferences {
  public:
    // Using map data type for storing probability for choosing a given receiver
    // with pointers to base class which Worker and Storehouse derive from
    using preferences_t = std::map<IPackageReceiver *, double>;

    using const_iterator = preferences_t::const_iterator;

    /**
     * @brief Constructor
     * @param pg Probability Generator (by default it's global from
     * "helpers.hpp")
     */
    explicit ReceiverPreferences(
        ProbabilityGenerator pg = probability_generator);

    /**
     * @brief Method for adding receivers
     * This method allows to keep the class constant - all probabilities
     * always sum to one, regardless of the amount of receivers
     */
    void add_receiver(IPackageReceiver *receiver);

    /**
     * @brief Method for removing receivers
     */
    void remove_receiver(IPackageReceiver *receiver);

    /**
     * @brief Picks a receiver based on probability distribution
     * @return pointer to the choosen receiver
     */
    IPackageReceiver *choose_receiver();

    /**
     * @brief Method for getting preferences map
     * Returns reference to the map, not copy
     * @return all current links, in "read-only" mode
     */
    const preferences_t &
    get_preferences() const; // & avoids copying the whole map object

    // Map iterators
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

  private:
    preferences_t preferences_; // map containing pointers and numbers
    ProbabilityGenerator pg_;
};

/**
 * @brief Base class for sending Nodes (Ramp, Worker)
 * Uses buffer
 */
class PackageSender {
  public:
    /**
     * @brief Default constructor
     */
    PackageSender() = default;

    /**
     * @brief Default Moving constructor
     */
    PackageSender(PackageSender &&) = default;

    /**
     * @brief Sends package from buffer to the choosen receiver
     * If buffer is empty, it does nothig
     */
    void send_package();

    /**
     * @brief Method for getting receiver preferences
     * @returns receiver preferences
     */
    const ReceiverPreferences &get_receiver_preferences() const;

    /**
     * @brief Gets sending buffer
     */
    const std::optional<Package>& get_sending_buffer() const { return buffer_; }

    // Non-const version to be able to modify
    ReceiverPreferences &get_receiver_preferences();

    // No destructor since both 'std::optional<Package>' and
    // 'ReceiverPreferences' are intelligent and clean up after themselves

  protected:
    /**
     * @brief Inserts package (all of its content due to r-reference &&) to the
     * output buffer
     */
    void push_package(Package &&package);

    std::optional<Package> buffer_; // Output buffer
    ReceiverPreferences
        receiver_preferences_; // ReceriverPreferences instance, containing
                               // preferences map for every object that derives
                               // from PackageSender base class
};

/**
 * @brief Interface for Nodes receiving packages
 * Base class for Worker and Storehouse
 * It's an interface because Worker and Storehouse storage packages in different
 * manner
 */
class IPackageReceiver {
  public:
    /**
     * @brief Handles receiving a product
     */
    virtual void receive_package(Package &&p) = 0;

    /**
     * @brief Gets ID of a Node
     */
    virtual ElementID get_id() const = 0;

    // Iterators for read-only, defined in "storage_types.hpp"
    using const_iterator = IPackageStockpile::const_iterator;

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;

    virtual ~IPackageReceiver() = default;

    /**
     * @brief Gets the type of a the receiver (Worker or Storehouse)
     */
    virtual ReceiverType get_receiver_type() const = 0;
};

/**
 * @brief Class representing a ramp, source of products
 * It doesn't need a queue for delivered products, products are delivered on
 * ramp one by one and instantly moved further - to workers
 */
class Ramp : public PackageSender {
  public:
    // No default constructor since it doesn't make sense for nodes to be empty

    /**
     * @brief Constructor setting id and offset (read from input file)
     * TimeOffset represents time between product deliveries
     */
    explicit Ramp(ElementID id, TimeOffset di);
    // explicit for clarity, but it's not necessary with multi-argument
    // constructors, complilator will not convert types anyways

    /**
     * @brief Method for delivering goods with a set frequency
     * This method is called in every round of the simulation
     * @arg is a current time of the simulation
     */
    void deliver_goods(Time t);

    /**
     * @brief Gets Ramp ID
     */
    ElementID get_id() const;

    /**
     * @brief Gets delivery interval
     */
    TimeOffset get_delivery_interval() const;

  private:
    ElementID id_;
    TimeOffset delivery_interval_;
};

/**
 * @brief Class representing a worker, processes products
 *
 */
class Worker : public PackageSender, public IPackageReceiver {
  public:
    /**
     * @brief Constructor setting id, offset and queue type
     */
    explicit Worker(ElementID id, TimeOffset pd,
                    std::unique_ptr<IPackageQueue> q);

    // AS A RECEIVER

    void receive_package(Package &&p) override;

    ReceiverType get_receiver_type() const override;

    // AS A WORKER

    /**
     * @brief Method for doing work by the Worker
     * This method is called in every round of the simulation (in
     * "processing" phase)
     * @arg t represents current time of the simulation, for the worker to
     * know when to finish processing current product
     */
    void do_work(Time t);

    // GETTERS

    /**
     * @brief Gets worker ID
     */
    ElementID get_id() const;

    /**
     * @brief Gets product processing duration
     */
    TimeOffset get_processing_duration() const;

    /**
     * @brief Gets product processing start time
     */
    Time get_product_processing_start_time() const;

    /**
     * @brief Gets worker queue type
     */
    IPackageQueue* get_queue() const;

    /**
     * @brief Gets the package being currently processed
     */
    const std::optional<Package>& get_processing_buffer() const { return processing_buffer_; }

    // ITERATORS
    const_iterator begin() const override;
    const_iterator end() const override;
    const_iterator cbegin() const override;
    const_iterator cend() const override;

  private:
    ElementID id_;
    TimeOffset processing_duration_;
    Time package_processing_start_time_ = 0;

    std::unique_ptr<IPackageQueue> q_; // Input queue
    std::optional<Package>
        processing_buffer_; // Product being current processed
};

/**
 * @brief Class representing a Storehouse
 */
class Storehouse : public IPackageReceiver {
  public:
    /**
     * @brief Constructor assigning ID and the object storage type (default is
     * FIFO)
     * @param id Unique Storehouse identifier
     * @param d Unique pointer to the storage implementation (stockpile)
     * Storehouse takes ownership of this pointer
     */
    explicit Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d =
                                          std::make_unique<PackageQueue>(
                                              PackageQueueType::FIFO));

    void receive_package(Package &&p) override;

    ReceiverType get_receiver_type() const override;

    ElementID get_id() const override;

    // Iterators implementation
    const_iterator begin() const override;
    const_iterator end() const override;
    const_iterator cbegin() const override;
    const_iterator cend() const override;

  private:
    ElementID id_;
    std::unique_ptr<IPackageStockpile> d_; // Container for packages
};

} // namespace NetSim