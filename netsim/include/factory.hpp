
#pragma once

#include <algorithm>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "nodes.hpp"
#include "types.hpp" // REMEMBER TO INCLUDE TYPES WHERE NEEDED
namespace NetSim {
/**
 * @brief Template for nodes collection
 * Std::list avoids problem with pointers when removing nodes
 * Templates must have an entire implementation in this header file
 */
template <typename Node>

/**
 * @brief Class for storing collection of Nodes of one type
 */
class NodeCollection {
  public:
    // Type alliases
    using container_t = std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator =
        typename container_t::const_iterator; // used by const methods, cannot
                                              // modify elements

    // Access methods
    iterator begin() { return container_.begin(); }
    iterator end() { return container_.end(); }
    const_iterator begin() const { return container_.begin(); }
    const_iterator end() const { return container_.end(); }
    const_iterator cbegin() const { return container_.cbegin(); }
    const_iterator cend() const { return container_.cend(); }

    /**
     * @brief Adds node to the collection (container)
     * Takes ownership (move)
     */
    void add(Node &&node) { container_.emplace_back(std::move(node)); }

    /**
     * @brief Removes node from the collection (container)
     */
    void remove_by_id(ElementID id) {
        auto it = find_by_id(id);
        if (it != container_.end()) {
            container_.erase(it);
        }
    }

    /**
     * @brief Finds an element in the container by the id
     * Uses lambda to compare all Nodes' IDs with a given ID
     * @returns pointer to the correct Node
     */
    iterator find_by_id(ElementID id) {
        return std::find_if(
            container_.begin(), container_.end(),
            [id](const Node &node) { return node.get_id() == id; });
    }

    /**
     * @brief Finds an element in the container by the id, read-only mode
     * Include CONST keyword at the end of function signature
     */
    const_iterator find_by_id(ElementID id) const {
        return std::find_if(
            container_.cbegin(), container_.cend(),
            [id](const Node &node) { return node.get_id() == id; });
    }

  private:
    container_t container_;
};

enum class ElementType {
    LOADING_RAMP, WORKER, STOREHOUSE, LINK
};

struct ParsedLineData {
    ElementType element_type;
    std::map<std::string, std::string> parameters;
};

ParsedLineData parse_line(std::string line);

/**
 * @brief Class managing the whole Net
 */
class Factory {
  public:
    // STRUCTURE MANAGEMENT METHODS

    // RAMPS
    /**
     * @brief Adds ramp to the Net
     */
    void add_ramp(Ramp &&r) { ramps_.add(std::move(r)); }

    /**
     * @brief Removes ramp from the Net
     */
    void remove_ramp(ElementID id) { ramps_.remove_by_id(id); }

    /**
     * @brief Finds ramp by ID
     * Delegating method
     * Exposes private fields to read-only mode
     * calls find_by_id function defined in NodeCollection class
     */
    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) {
        return ramps_.find_by_id(id);
    }

    /**
     * @brief Finds ramp by ID (CONST version)
     */
    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const {
        return ramps_.find_by_id(id);
    }

    /**
     * @brief Ramp iterators
     */
    NodeCollection<Ramp>::const_iterator ramp_cbegin() const {
        return ramps_.cbegin();
    }
    NodeCollection<Ramp>::const_iterator ramp_cend() const {
        return ramps_.cend();
    }

    // WORKERS
    /**
     * @brief Adds worker to the Net
     */
    void add_worker(Worker &&w) { workers_.add(std::move(w)); }

    /**
     * @brief Removes worker from the Net
     * Complex logic, implemented in .cpp
     */
    void remove_worker(ElementID id);

    /**
     * @brief Finds worker by ID
     * Delegating method
     * Exposes private fields to read-only mode
     * calls find_by_id function defined in NodeCollection class
     */
    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) {
        return workers_.find_by_id(id);
    }

    /**
     * @brief Finds worker by ID (CONST version)
     */
    NodeCollection<Worker>::const_iterator
    find_worker_by_id(ElementID id) const {
        return workers_.find_by_id(id);
    }

    /**
     * @brief Worker iterators
     */
    NodeCollection<Worker>::const_iterator worker_cbegin() const {
        return workers_.cbegin();
    }
    NodeCollection<Worker>::const_iterator worker_cend() const {
        return workers_.cend();
    }

    // STOREHOUSES
    /**
     * @brief Adds storehouse to the Net
     */
    void add_storehouse(Storehouse &&s) { storehouses_.add(std::move(s)); }

    /**
     * @brief Removes storehouse from the Net
     * Complex logic, implemented in .cpp
     */
    void remove_storehouse(ElementID id); // Złożona logika -> w .cpp

    /**
     * @brief Finds storehouse by ID
     * Delegating method
     * Exposes private fields to read-only mode
     * calls find_by_id function defined in NodeCollection class
     */
    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) {
        return storehouses_.find_by_id(id);
    }

    /**
     * @brief Finds storehouse by ID (CONST version)
     */
    NodeCollection<Storehouse>::const_iterator
    find_storehouse_by_id(ElementID id) const {
        return storehouses_.find_by_id(id);
    }

    /**
     * @brief Storehouses iterators
     */
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const {
        return storehouses_.cbegin();
    }
    NodeCollection<Storehouse>::const_iterator storehouse_cend() const {
        return storehouses_.cend();
    }

    // VERIFICATION AND SIMULATION METHODS

    /**
     * @brief Checks if the Net is consistent
     * All ramps must have a link to at least one storage
     * Implements the main DSF algorithm
     */
    bool is_consistent();

    /**
     * @brief Activates delivery processes in the simulation
     */
    void do_deliveries(Time t);

    /**
     * @brief Activates package passing processes in the simulation
     */
    void do_package_passing();

    /**
     * @brief Activates workers work processes in the simulation
     */
    void do_work(Time t);

  private:
    /**
     * @brief Helper function for removing receiver from receiver_preverence
     * lists of other senders
     */
    template <typename Node>
    void remove_receiver(NodeCollection<Node> &collection, ElementID id) {
        auto it = collection.find_by_id(id);
        if (it != collection.end()) {
            IPackageReceiver* receiver = dynamic_cast<IPackageReceiver*>(&(*it));
            if (receiver) {
                for (auto& ramp : ramps_) {
                    ramp.get_receiver_preferences().remove_receiver(receiver);
                }
                for (auto& worker : workers_) {
                    worker.get_receiver_preferences().remove_receiver(receiver);
                }
            }
            collection.remove_by_id(id);
        }
    }

    NodeCollection<Ramp> ramps_;
    NodeCollection<Worker> workers_;
    NodeCollection<Storehouse> storehouses_;
};

Factory load_factory_structure(std::istream &is);

void save_factory_structure(const Factory &factory, std::ostream &os);

} // namespace NetSim
