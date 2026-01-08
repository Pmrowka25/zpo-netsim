# 00. Selection of std::deque over std::list for Package Storage

## Status
Accepted

## Context
The project requirements suggest using `std::list` for implementing package queues (`PackageQueue`). The primary argument provided in the requirements is to avoid "iterator and reference invalidation" when adding or removing elements.

However, modern C++ performance characteristics and the specific access patterns of the `NetSim` simulation require a re-evaluation of this suggestion. The simulation involves:
1. Frequent insertions at one end and removals from either end (FIFO/LIFO).
2. Iterating over the container for reporting purposes.
3. Strict ownership management (Packages are moved, not referenced via long-lived pointers from outside the queue).

## Decision
We have decided to use **`std::deque` (Double-Ended Queue)** as the underlying container for `PackageQueue` and the basis for `IPackageStockpile` iterators.

## Consequences & Justification

### 1. Iterator Invalidation is Irrelevant for our Use Case
The argument for `std::list` relies on the scenario where external pointers to elements *inside* the queue must remain valid while the queue is modified. In our system:
* We use **Value Semantics** and **Move Semantics**. When a `Worker` processes a package, they `pop` (remove) it from the queue entirely.
* We do not hold pointers to elements residing inside the queue across simulation turns.
* Therefore, the stability of memory addresses of elements inside the container is not required.

### 2. Cache Locality and Performance
* **`std::list` (Linked List):** Allocates each node separately on the heap. Traversing the list (for reports) causes frequent cache misses (jumping around memory).
* **`std::deque`:** Allocates memory in chunks (blocks). Traversing it is significantly faster due to better cache locality.
* **Overhead:** `std::list` has a memory overhead of 2 pointers (prev/next) per element. `std::deque` overhead is minimal.

### 3. Implementation Details
The `IPackageStockpile` interface will define `const_iterator` based on `std::deque<Package>::const_iterator`.