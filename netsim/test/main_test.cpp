#include "package.hpp"
#include "storage_types.hpp"
#include <cassert>
#include <iostream>

using namespace NetSim;

int main() {
  // Test 1: Package ID generation
  //
  /*
  Package p1;
  Package p2;
  std::cout << "P1 ID: " << p1.get_id() << std::endl;
  std::cout << "P2 ID: " << p2.get_id() << std::endl;
  assert(p1.get_id() != p2.get_id());
  */

  // Test 2: Queue FIFO
  PackageQueue q_fifo(PackageQueueType::FIFO);
  q_fifo.push(Package(1));
  q_fifo.push(Package(2));

  Package p_out1 = q_fifo.pop();
  std::cout << "FIFO Pop 1 (expect 1): " << p_out1.get_id() << std::endl;
  assert(p_out1.get_id() == 1);

  // Test 3: Queue LIFO
  PackageQueue q_lifo(PackageQueueType::LIFO);
  q_lifo.push(Package(1));
  q_lifo.push(Package(2));

  Package p_out2 = q_lifo.pop();
  std::cout << "LIFO Pop 1 (expect 2): " << p_out2.get_id() << std::endl;
  assert(p_out2.get_id() == 2);

  std::cout << "All basic tests passed!" << std::endl;
  return 0;
}