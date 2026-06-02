#include <catch2/catch_test_macros.hpp>
#include <pluma/Render/DummyRenderer.hpp>
#include <pluma/Render/DisplayList.hpp>
#include <pluma/Typography/DummyTypography.hpp>

using namespace pluma;

TEST_CASE("Renderer execution from DisplayList", "[render]") {
    DisplayList dlist;
    
    dlist.addCommand(std::make_unique<FillRectCommand>(
        Rect{Twips(0), Twips(0), Twips(100), Twips(100)}, 
        0xFFFF0000
    ));

    ShapedTextRun run;
    run.glyphs.push_back({}); // 1 dummy glyph
    dlist.addCommand(std::make_unique<DrawGlyphRunCommand>(
        Rect{Twips(10), Twips(10), Twips(50), Twips(20)},
        std::move(run),
        "test",
        std::make_shared<DummyFont>(FontDescriptor{}),
        0xFF000000
    ));

    dlist.addCommand(std::make_unique<PushClipCommand>(Rect{Twips(0), Twips(0), Twips(50), Twips(50)}));
    dlist.addCommand(std::make_unique<PopClipCommand>());

    DummyRenderer renderer;

    // Execute the commands manually for test purposes
    for (const auto& cmd : dlist.getCommands()) {
        cmd->execute(renderer);
    }

    REQUIRE(renderer.log.size() == 4);
    REQUIRE(renderer.log[0] == "drawRect 0 0");
    REQUIRE(renderer.log[1] == "drawGlyphRun 1 glyphs");
    REQUIRE(renderer.log[2] == "pushClip");
    REQUIRE(renderer.log[3] == "popClip");
}
