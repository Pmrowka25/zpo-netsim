#include <gtest/gtest.h>
#include "factory.hpp"
#include "helpers.hpp"
#include "nodes.hpp"
#include "package.hpp"
#include "reports.hpp"
#include "simulation.hpp"
#include "storage_types.hpp"

using namespace NetSim;

// --- PACKAGE AND QUEUE TESTS ---

TEST(PackageTest, IDGenerationIsUnique) {
    Package p1;
    Package p2;
    EXPECT_NE(p1.get_id(), p2.get_id());
}

TEST(PackageQueueTest, FIFO_Order) {
    PackageQueue q(PackageQueueType::FIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p1 = q.pop();
    EXPECT_EQ(p1.get_id(), 1); // FIFO: First in, first out

    Package p2 = q.pop();
    EXPECT_EQ(p2.get_id(), 2);
}

TEST(PackageQueueTest, LIFO_Order) {
    PackageQueue q(PackageQueueType::LIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p1 = q.pop();
    EXPECT_EQ(p1.get_id(), 2); // LIFO: Last in, first out (stack)

    Package p2 = q.pop();
    EXPECT_EQ(p2.get_id(), 1);
}

// --- BUSINESS LOGIC TESTS (NODES) ---

// Helper for testing PackageSender (access to protected members)
class TestSender : public PackageSender {
public:
    // Wrapper exposing the push_package method
    void push_package_public(Package&& p) {
        push_package(std::move(p));
    }
    // Check if buffer is empty
    bool is_buffer_empty() const {
        return !buffer_.has_value();
    }
};

TEST(PackageSenderTest, BufferClearedAfterSend) {
    TestSender sender;
    
    // Create a "Mock" receiver so the sender has somewhere to send to
    Storehouse receiver(1); 
    sender.get_receiver_preferences().add_receiver(&receiver);

    // Insert a package
    sender.push_package_public(Package(100));
    EXPECT_FALSE(sender.is_buffer_empty());

    // Send
    sender.send_package();

    // Check: buffer should be empty, and package in storehouse
    EXPECT_TRUE(sender.is_buffer_empty());
    EXPECT_EQ(receiver.begin()->get_id(), 100);
}

TEST(ReceiverPreferencesTest, ProbabilityScaling) {
    ReceiverPreferences prefs;
    Storehouse s1(1), s2(2);

    prefs.add_receiver(&s1);
    // 1 receiver = 1.0
    EXPECT_DOUBLE_EQ(prefs.get_preferences().at(&s1), 1.0);

    prefs.add_receiver(&s2);
    // 2 receivers = 0.5 each
    EXPECT_DOUBLE_EQ(prefs.get_preferences().at(&s1), 0.5);
    EXPECT_DOUBLE_EQ(prefs.get_preferences().at(&s2), 0.5);

    prefs.remove_receiver(&s1);
    // Back to 1.0
    EXPECT_DOUBLE_EQ(prefs.get_preferences().at(&s2), 1.0);
}

TEST(ReceiverPreferencesTest, MockedGeneratorSelection) {
    // Define a generator that always returns 0.3
    auto fixed_gen = []() { return 0.3; };
    ReceiverPreferences prefs(fixed_gen);

    Storehouse s1(1), s2(2);
    prefs.add_receiver(&s1); 
    prefs.add_receiver(&s2); 

    // std::map is sorted by key (pointer address). 
    // We expect the first receiver in the map to be selected because p=0.3 < 0.5.
    IPackageReceiver* expected = prefs.get_preferences().begin()->first;

    IPackageReceiver* selected = prefs.choose_receiver();
    EXPECT_EQ(selected, expected);
}

TEST(RampTest, DeliveryInCorrectRound) {
    // Ramp delivers every 2 rounds (interval 2)
    Ramp ramp(1, 2); 
    Storehouse receiver(1);
    ramp.get_receiver_preferences().add_receiver(&receiver);

    // Round 1: (1-1)%2 == 0 -> SHOULD be a delivery
    ramp.deliver_goods(1);
    ramp.send_package(); // Ramp immediately sends from buffer
    EXPECT_FALSE(receiver.begin() == receiver.end()); // Storehouse not empty

    // Round 2: (2-1)%2 == 1 -> NO delivery
    // Clear storehouse to check if anything arrives
    Storehouse receiver2(2);
    ramp.get_receiver_preferences().remove_receiver(&receiver);
    ramp.get_receiver_preferences().add_receiver(&receiver2);
    
    ramp.deliver_goods(2);
    ramp.send_package();
    EXPECT_TRUE(receiver2.begin() == receiver2.end()); // Storehouse empty
}

TEST(WorkerTest, ProcessingDurationAndForwarding) {
    // Worker processes for 2 rounds
    Worker worker(1, 2, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    Storehouse store(1);
    worker.get_receiver_preferences().add_receiver(&store);

    // Give it a package
    worker.receive_package(Package(50));

    // Round 1: Takes package in hand (start of work)
    worker.do_work(1); 
    // Nothing should be sent, because work takes 2 rounds
    worker.send_package(); 
    EXPECT_TRUE(store.begin() == store.end()); // Storehouse empty

    // Round 2: Finishes work (1 + 2 - 1 = 2)
    worker.do_work(2);
    // Now the package should be in the worker's outgoing buffer. Sending.
    worker.send_package();
    
    // Check if it arrived
    ASSERT_FALSE(store.begin() == store.end());
    EXPECT_EQ(store.begin()->get_id(), 50);
}

TEST(StorehouseTest, ReceivingPackage) {
    Storehouse store(1);
    store.receive_package(Package(99));

    auto it = store.begin();
    ASSERT_NE(it, store.end()); // Cannot be empty
    EXPECT_EQ(it->get_id(), 99);
}

// --- SIMULATION AND REPORTING TESTS ---

TEST(IntervalReportNotifierTest, CorrectTurns) {
    IntervalReportNotifier notifier(2);
    EXPECT_TRUE(notifier.should_generate_report(1)); // 1, 3, 5...
    EXPECT_FALSE(notifier.should_generate_report(2));
    EXPECT_TRUE(notifier.should_generate_report(3));
}

TEST(SpecificTurnsReportNotifierTest, CorrectTurns) {
    SpecificTurnsReportNotifier notifier({1, 4});
    EXPECT_TRUE(notifier.should_generate_report(1));
    EXPECT_FALSE(notifier.should_generate_report(2));
    EXPECT_FALSE(notifier.should_generate_report(3));
    EXPECT_TRUE(notifier.should_generate_report(4));
}

TEST(SimulationTest, FullCycle) {
    Factory f;
    f.add_ramp(Ramp(1, 1));
    f.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    f.add_storehouse(Storehouse(1));
    
    f.find_ramp_by_id(1)->get_receiver_preferences().add_receiver(&*f.find_worker_by_id(1));
    f.find_worker_by_id(1)->get_receiver_preferences().add_receiver(&*f.find_storehouse_by_id(1));
    
    // T1: Deliver to Ramp, Send to Worker, Worker starts processing
    // T2: Worker finishes, Send to Storehouse
    simulate(f, 2, [](Factory&, Time) {});
    
    auto it = f.find_storehouse_by_id(1)->begin();
    EXPECT_NE(it, f.find_storehouse_by_id(1)->end());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
