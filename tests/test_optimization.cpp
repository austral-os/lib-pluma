#include <catch2/catch_test_macros.hpp>
#include <pluma/Optimization/ObjectPool.hpp>
#include <pluma/Optimization/ShaperCache.hpp>
#include <pluma/Typography/DummyTypography.hpp>
#include <chrono>

using namespace pluma;
using namespace pluma::optimization;

class MockNode {
public:
    int id;
    MockNode(int id) : id(id) {}
    ~MockNode() {}
};

TEST_CASE("Optimization - ObjectPool Allocation", "[optimization]") {
    auto& pool = ObjectPool<MockNode>::getInstance();
    
    auto node1 = pool.acquire(10);
    REQUIRE(node1->id == 10);
    
    MockNode* ptr = node1.get();
    node1.reset(); // Should return to pool
    
    auto node2 = pool.acquire(20);
    REQUIRE(node2->id == 20);
    REQUIRE(node2.get() == ptr); // Recycled pointer
}

TEST_CASE("Optimization - ShaperCache Efficiency", "[optimization]") {
    auto dummy_shaper = std::make_shared<DummyTextShaper>();
    ShaperCache cache(dummy_shaper);
    
    auto font_manager = std::make_shared<DummyFontManager>();
    FontDescriptor desc;
    auto font = font_manager->getFont(desc);
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        cache.shapeText("Hello World", font);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    // The time taken should be extremely small because of the cache
    REQUIRE(duration_ms < 10.0);
}
