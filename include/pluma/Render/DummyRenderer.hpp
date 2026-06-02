/**
 * @file DummyRenderer.hpp
 * @brief A headless dummy renderer for unit testing the pipeline.
 */
#pragma once

#include <vector>
#include <string>
#include <pluma/Render/IRenderer.hpp>

namespace pluma {

/**
 * @class DummyRenderer
 * @brief Records drawing calls into an observable log for test assertions.
 */
class DummyRenderer : public IRenderer {
public:
    std::vector<std::string> log;

    void drawRect(const Rect& rect, Color color) override {
        (void)color;
        log.push_back("drawRect " + std::to_string(rect.x.getValue()) + " " + std::to_string(rect.y.getValue()));
    }

    void drawGlyphRun(const Rect& rect, const ShapedTextRun& run, const std::string& text, std::shared_ptr<IFont> font, Color color) override {
        (void)rect;
        (void)text;
        (void)font;
        (void)color;
        log.push_back("drawGlyphRun " + std::to_string(run.glyphs.size()) + " glyphs");
    }

    void drawImage(const Rect& rect, const std::string& path) override {
        (void)rect;
        log.push_back("drawImage " + path);
    }

    void pushClip(const Rect& rect) override {
        (void)rect;
        log.push_back("pushClip");
    }

    void popClip() override {
        log.push_back("popClip");
    }
};

} // namespace pluma
