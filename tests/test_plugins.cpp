#include <catch2/catch_test_macros.hpp>
#include <pluma/Plugins/MarkdownImporter.hpp>
#include <pluma/Plugins/MarkdownExporter.hpp>
#include <pluma/Plugins/PdfExporter.hpp>
#include <pluma/Plugins/PlumaArchiveExporter.hpp>
#include <pluma/Plugins/PlumaArchiveImporter.hpp>
#include <pluma/PlumaEditor.hpp>
#include <pluma/Typography/DummyTypography.hpp>

using namespace pluma;
using namespace pluma::plugins;

TEST_CASE("Plugins - Markdown Import and Export", "[plugins]") {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font_manager = std::make_shared<DummyFontManager>();

    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f;
    auto font = font_manager->getFont(desc);

    PlumaEditor editor(shaper, font);

    std::string markdown_content = "# Hello World\n\nThis is a test of the markdown plugin.";

    MarkdownImporter importer;
    bool success = importer.import(markdown_content, editor);
    
    REQUIRE(success);
    REQUIRE(editor.getText() == markdown_content);

    // Now export Markdown
    MarkdownExporter exporter;
    std::string exported = exporter.exportDoc(editor.getSnapshot());
    REQUIRE(exported == markdown_content);

    // Now export PDF
    PdfExporter pdf_exporter;
    bool pdf_success = pdf_exporter.exportToFile("test_output.pdf", editor);
    REQUIRE(pdf_success);
}

TEST_CASE("Plugins - Native Pluma Archive (.pluma)", "[plugins]") {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font_manager = std::make_shared<DummyFontManager>();

    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f;
    auto font = font_manager->getFont(desc);

    PlumaEditor editor(shaper, font);

    // 1. Create a document with text and styles
    editor.loadText("Hello Beautiful World");
    // Apply some styles: "Beautiful" is from 6 to 15
    editor.applyStyle(6, 9, PropertyId::TextColor, static_cast<Color>(0xFFFF0000)); // Red
    editor.applyStyle(6, 9, PropertyId::FontWeight, static_cast<uint16_t>(700)); // Bold

    // 2. Export to .pluma
    PlumaArchiveExporter exporter;
    bool export_ok = exporter.exportToFile("test_archive.pluma", editor);
    REQUIRE(export_ok);

    // 3. Import to a new editor
    PlumaEditor editor2(shaper, font);
    PlumaArchiveImporter importer;
    bool import_ok = importer.importFile("test_archive.pluma", editor2);
    REQUIRE(import_ok);

    // 4. Verify contents
    REQUIRE(editor2.getText() == "Hello Beautiful World");
    
    // Verify styles
    const auto& registry = editor2.getFormatRegistry();
    auto spans = registry.getSpans();
    REQUIRE(spans.size() == 2);
    REQUIRE(spans[0].start == 6);
    REQUIRE(spans[0].length == 9);
    REQUIRE(spans[1].start == 6);
    REQUIRE(spans[1].length == 9);
    
    // Check first span (color)
    auto color_opt = spans[0].style.get(PropertyId::TextColor);
    REQUIRE(color_opt.has_value());
    REQUIRE(std::get<Color>(*color_opt) == 0xFFFF0000);
    
    // Check second span (weight)
    auto weight_opt = spans[1].style.get(PropertyId::FontWeight);
    REQUIRE(weight_opt.has_value());
    REQUIRE(std::get<uint16_t>(*weight_opt) == 700);
}
