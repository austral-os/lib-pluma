#include <catch2/catch_test_macros.hpp>
#include <pluma/PieceTable.hpp>
#include <pluma/Services/ServiceManager.hpp>
#include <pluma/Services/SearchIndexer.hpp>

using namespace pluma;

TEST_CASE("Document Services - Asynchronous Analysis", "[services]") {
    PieceTable table;
    table.insert(0, "The quick brown fox jumps over the lazy fox.");

    auto snapshot = table.createSnapshot();

    ServiceManager manager;
    auto indexer = std::make_shared<SearchIndexer>();
    
    manager.registerService(indexer);
    
    // Dispatch asynchronous task
    manager.runAnalysis(snapshot);
    
    // Wait for the background thread to finish
    manager.waitForAll();
    
    // Validate that the indexer successfully counted words asynchronously
    REQUIRE(indexer->getWordFrequency("fox") == 2);
    REQUIRE(indexer->getWordFrequency("the") == 2);
    REQUIRE(indexer->getWordFrequency("lazy") == 1);
    REQUIRE(indexer->getWordFrequency("wolf") == 0);
}
