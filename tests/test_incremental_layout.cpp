#include <catch2/catch_test_macros.hpp>
#include <pluma/Layout/DirtyRegion.hpp>
#include <pluma/Layout/LayoutScheduler.hpp>

using namespace pluma;

TEST_CASE("DirtyRegion and InvalidationTracker", "[layout]") {
    InvalidationTracker tracker;
    
    REQUIRE_FALSE(tracker.hasDirtyRegions());
    
    tracker.markDirty(10, 5); // 10 to 15
    REQUIRE(tracker.hasDirtyRegions());
    REQUIRE(tracker.getRegions().size() == 1);
    
    DirtyRegion r1{10, 5};
    DirtyRegion r2{12, 5}; // 12 to 17
    DirtyRegion r3{20, 5}; // 20 to 25
    
    REQUIRE(r1.overlaps(r2));
    REQUIRE_FALSE(r1.overlaps(r3));
    
    tracker.clear();
    REQUIRE_FALSE(tracker.hasDirtyRegions());
}

TEST_CASE("LayoutScheduler priority and cancellation", "[layout]") {
    LayoutScheduler scheduler;
    
    int execution_order = 0;
    int high_priority_result = 0;
    int low_priority_result = 0;
    
    uint32_t id_low = scheduler.scheduleTask(TaskPriority::Low, [&]() {
        low_priority_result = ++execution_order;
    });
    
    scheduler.scheduleTask(TaskPriority::High, [&]() {
        high_priority_result = ++execution_order;
    });
    
    REQUIRE(scheduler.hasPendingTasks());
    
    // Cancel the low priority task
    scheduler.cancelTask(id_low);
    
    scheduler.processAll();
    
    REQUIRE_FALSE(scheduler.hasPendingTasks());
    
    // High priority should have executed first (and only, since low was cancelled)
    REQUIRE(high_priority_result == 1);
    REQUIRE(low_priority_result == 0); // Never executed
}

TEST_CASE("LayoutScheduler multiple tasks", "[layout]") {
    LayoutScheduler scheduler;
    
    std::vector<int> results;
    
    scheduler.scheduleTask(TaskPriority::Low, [&]() { results.push_back(3); });
    scheduler.scheduleTask(TaskPriority::High, [&]() { results.push_back(1); });
    scheduler.scheduleTask(TaskPriority::Medium, [&]() { results.push_back(2); });
    
    scheduler.processAll();
    
    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == 1); // High
    REQUIRE(results[1] == 2); // Medium
    REQUIRE(results[2] == 3); // Low
}
