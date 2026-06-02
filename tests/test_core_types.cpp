#include <catch2/catch_test_macros.hpp>
#include <pluma/NodeId.hpp>
#include <pluma/RevisionId.hpp>
#include <pluma/CoreTypes.hpp>

using namespace pluma;

TEST_CASE("NodeId basic operations", "[core]") {
    NodeId id1(42);
    NodeId id2(42);
    NodeId id3(100);

    REQUIRE(id1.getValue() == 42);
    REQUIRE(id1 == id2);
    REQUIRE(id1 != id3);
}

TEST_CASE("RevisionId basic operations", "[core]") {
    RevisionId rev1(1);
    RevisionId rev2(1);
    RevisionId rev3(5);

    REQUIRE(rev1.getValue() == 1);
    REQUIRE(rev1 == rev2);
    REQUIRE(rev1 != rev3);
}

TEST_CASE("Twips arithmetic and comparisons", "[core]") {
    Twips t1(1440); // 1 inch
    Twips t2(720);  // 0.5 inch
    
    REQUIRE(t1.getValue() == 1440);
    REQUIRE(t2.getValue() == 720);
    
    REQUIRE(t1 != t2);
    REQUIRE(t1 == Twips(1440));
    
    Twips t3 = t1 + t2;
    REQUIRE(t3.getValue() == 2160);
    
    Twips t4 = t1 - t2;
    REQUIRE(t4.getValue() == 720);
    REQUIRE(t4 == t2);
}

TEST_CASE("Point and Rect layout", "[core]") {
    Point p{Twips(10), Twips(20)};
    REQUIRE(p.x.getValue() == 10);
    REQUIRE(p.y.getValue() == 20);

    Rect r{Twips(0), Twips(0), Twips(100), Twips(200)};
    REQUIRE(r.x.getValue() == 0);
    REQUIRE(r.y.getValue() == 0);
    REQUIRE(r.width.getValue() == 100);
    REQUIRE(r.height.getValue() == 200);
}
