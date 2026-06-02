#include <catch2/catch_test_macros.hpp>
#include <pluma/Diagnostics/Dumper.hpp>
#include <pluma/Diagnostics/Profiler.hpp>
#include <pluma/DOM/DOMManager.hpp>
#include <pluma/PieceTable.hpp>

using namespace pluma;
using namespace pluma::diagnostics;

TEST_CASE("Diagnostics - DOM Dumper", "[diagnostics]") {
    PieceTable table;
    table.insert(0, "Test string");

    DOMManager dom_manager;
    auto root = dom_manager.rebuild(table);

    std::string dump = Dumper::dumpDOM(*root);
    
    // The dump should contain string references to the nodes
    REQUIRE(dump.find("DOMNode[Document]") != std::string::npos);
    REQUIRE(dump.find("DOMNode[Paragraph]") != std::string::npos);
    REQUIRE(dump.find("DOMNode[Run, offset: 0, len: 11]") != std::string::npos);
}

TEST_CASE("Diagnostics - Profiler Tracking", "[diagnostics]") {
    Profiler::getInstance().clear();

    {
        PLUMA_PROFILE_SCOPE("TestScope");
        // Simulate some work
        int sum = 0;
        for (int i = 0; i < 10000; i++) { sum += i; }
        (void)sum;
    }

    std::string report = Profiler::getInstance().getReport();
    
    REQUIRE(report.find("TestScope") != std::string::npos);
    // At least 1 call
    REQUIRE(report.find("1") != std::string::npos);
}
