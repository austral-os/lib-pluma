/**
 * @file DisplayCommand.hpp
 * @brief Defines the DisplayCommand interface and concrete render commands.
 */
#pragma once

#include <memory>
#include <vector>
#include <pluma/CoreTypes.hpp>
#include <pluma/Typography/Glyph.hpp>
#include <pluma/Style/StyleProperties.hpp>
#include <pluma/Typography/IFontManager.hpp>
#include <pluma/Render/IRenderer.hpp>

namespace pluma {

/**
 * @enum CommandType
 * @brief Categorizes drawing commands for the renderer.
 */
enum class CommandType {
    FillRect,
    DrawLine,
    DrawGlyphRun,
    DrawImage,
    PushClip,
    PopClip
};

/**
 * @class DisplayCommand
 * @brief Abstract base class for drawing operations in the display list.
 */
class DisplayCommand {
public:
    virtual ~DisplayCommand() = default;

    /**
     * @brief Gets the type of the display command.
     * @return The CommandType.
     */
    virtual CommandType getType() const = 0;

    /**
     * @brief Gets the bounding box affected by this command.
     * @return The bounding Rect.
     */
    virtual Rect getBounds() const = 0;

    /**
     * @brief Executes this command against a generic renderer.
     * @param renderer The target renderer instance.
     */
    virtual void execute(IRenderer& renderer) const = 0;
};

/**
 * @class FillRectCommand
 * @brief Command to draw a solid rectangle.
 */
class FillRectCommand : public DisplayCommand {
public:
    FillRectCommand(const Rect& rect, Color color)
        : rect_(rect), color_(color) {}

    CommandType getType() const override { return CommandType::FillRect; }
    Rect getBounds() const override { return rect_; }
    void execute(IRenderer& renderer) const override { renderer.drawRect(rect_, color_); }

    Color getColor() const { return color_; }

private:
    Rect rect_;
    Color color_;
};

/**
 * @class DrawLineCommand
 * @brief Command to draw a line with a specific style.
 */
class DrawLineCommand : public DisplayCommand {
public:
    DrawLineCommand(const Rect& rect, const Point& start, const Point& end, Twips thickness, Color color, int line_style)
        : rect_(rect), start_(start), end_(end), thickness_(thickness), color_(color), line_style_(line_style) {}

    CommandType getType() const override { return CommandType::DrawLine; }
    Rect getBounds() const override { return rect_; }
    void execute(IRenderer& renderer) const override { renderer.drawLine(start_, end_, thickness_, color_, line_style_); }

    Point getStart() const { return start_; }
    Point getEnd() const { return end_; }
    Twips getThickness() const { return thickness_; }
    Color getColor() const { return color_; }
    int getLineStyle() const { return line_style_; }

private:
    Rect rect_;
    Point start_;
    Point end_;
    Twips thickness_;
    Color color_;
    int line_style_;
};

/**
 * @class DrawGlyphRunCommand
 * @brief Command to draw a sequence of shaped text.
 */
class DrawGlyphRunCommand : public DisplayCommand {
public:
    DrawGlyphRunCommand(const Rect& rect, ShapedTextRun run, std::string text, std::shared_ptr<IFont> font, Color color)
        : rect_(rect), run_(std::move(run)), text_(std::move(text)), font_(std::move(font)), color_(color) {}

    CommandType getType() const override { return CommandType::DrawGlyphRun; }
    Rect getBounds() const override { return rect_; }
    void execute(IRenderer& renderer) const override { renderer.drawGlyphRun(rect_, run_, text_, font_, color_); }

    const ShapedTextRun& getRun() const { return run_; }
    const std::string& getText() const { return text_; }
    std::shared_ptr<IFont> getFont() const { return font_; }
    Color getColor() const { return color_; }

private:
    Rect rect_;
    ShapedTextRun run_;
    std::string text_;
    std::shared_ptr<IFont> font_;
    Color color_;
};

/**
 * @class DrawImageCommand
 * @brief Command to draw an image.
 */
class DrawImageCommand : public DisplayCommand {
public:
    DrawImageCommand(const Rect& rect, std::string path)
        : rect_(rect), path_(std::move(path)) {}

    CommandType getType() const override { return CommandType::DrawImage; }
    Rect getBounds() const override { return rect_; }
    void execute(IRenderer& renderer) const override { renderer.drawImage(rect_, path_); }

    const std::string& getPath() const { return path_; }

private:
    Rect rect_;
    std::string path_;
};

/**
 * @class PushClipCommand
 * @brief Command to restrict rendering to a specific rectangle.
 */
class PushClipCommand : public DisplayCommand {
public:
    explicit PushClipCommand(const Rect& rect)
        : rect_(rect) {}

    CommandType getType() const override { return CommandType::PushClip; }
    Rect getBounds() const override { return rect_; }
    void execute(IRenderer& renderer) const override { renderer.pushClip(rect_); }

private:
    Rect rect_;
};

/**
 * @class PopClipCommand
 * @brief Command to remove the last applied clipping rectangle.
 */
class PopClipCommand : public DisplayCommand {
public:
    PopClipCommand() = default;

    CommandType getType() const override { return CommandType::PopClip; }
    void execute(IRenderer& renderer) const override { renderer.popClip(); }
    
    // Popping a clip doesn't have a specific rendering bound,
    // so we return an infinitely large rect to ensure it isn't culled incorrectly.
    Rect getBounds() const override { 
        return {Twips(-1000000), Twips(-1000000), Twips(2000000), Twips(2000000)}; 
    }
};

} // namespace pluma
