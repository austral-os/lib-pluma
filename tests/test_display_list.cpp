#include <catch2/catch_test_macros.hpp>
#include <pluma/Render/DisplayList.hpp>
#include <pluma/Typography/DummyTypography.hpp>

using namespace pluma;

TEST_CASE("DisplayList and Viewport Culling", "[render]") {
    DisplayList dlist;

    // Command 1: FillRect at (0, 0, 100, 100)
    dlist.addCommand(std::make_unique<FillRectCommand>(
        Rect{Twips(0), Twips(0), Twips(100), Twips(100)}, 
        0xFFFF0000
    ));

    // Command 2: FillRect at (200, 200, 50, 50)
    dlist.addCommand(std::make_unique<FillRectCommand>(
        Rect{Twips(200), Twips(200), Twips(50), Twips(50)}, 
        0xFF00FF00
    ));

    // Command 3: PushClip at (50, 50, 200, 200)
    dlist.addCommand(std::make_unique<PushClipCommand>(
        Rect{Twips(50), Twips(50), Twips(200), Twips(200)}
    ));

    REQUIRE(dlist.getCommands().size() == 3);

    SECTION("Viewport intersecting everything") {
        Rect viewport{Twips(0), Twips(0), Twips(1000), Twips(1000)};
        auto culled = dlist.getCommandsInRect(viewport);
        
        REQUIRE(culled.size() == 3);
    }

    SECTION("Viewport intersecting only first rect") {
        Rect viewport{Twips(0), Twips(0), Twips(50), Twips(50)};
        auto culled = dlist.getCommandsInRect(viewport);
        
        REQUIRE(culled.size() == 1);
        REQUIRE(culled[0]->getType() == CommandType::FillRect);
        
        auto* fill = static_cast<const FillRectCommand*>(culled[0]);
        REQUIRE(fill->getColor() == 0xFFFF0000);
    }

    SECTION("Viewport intersecting nothing") {
        Rect viewport{Twips(500), Twips(500), Twips(10), Twips(10)};
        auto culled = dlist.getCommandsInRect(viewport);
        
        REQUIRE(culled.size() == 0);
    }

    SECTION("Viewport intersecting only clip") {
        Rect viewport{Twips(150), Twips(150), Twips(10), Twips(10)};
        auto culled = dlist.getCommandsInRect(viewport);
        
        REQUIRE(culled.size() == 1);
        REQUIRE(culled[0]->getType() == CommandType::PushClip);
    }
}
