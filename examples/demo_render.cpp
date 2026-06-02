#include <cairo/cairo.h>
#include <iostream>
#include <pluma/PlumaEditor.hpp>
#include <pluma/Render/CairoRenderer.hpp>
#include <pluma/Typography/DummyTypography.hpp>
#include <pluma/Plugins/PdfExporter.hpp>

using namespace pluma;

class RealCairoShaper : public ITextShaper {
public:
  RealCairoShaper() {
    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cr_ = cairo_create(surface);
    cairo_select_font_face(cr_, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr_, 16.0);
    cairo_surface_destroy(surface);
  }

  ~RealCairoShaper() { cairo_destroy(cr_); }

  ShapedTextRun shapeText(std::string_view text,
                          const std::shared_ptr<IFont> &font) override {
    float size_pt = font->getDescriptor().size_pt;
    cairo_set_font_size(cr_, size_pt);
    
    cairo_text_extents_t extents;
    cairo_text_extents(cr_, std::string(text).c_str(), &extents);

    ShapedTextRun run;
    run.total_width = Twips(extents.x_advance * 15.0);
    run.max_ascent = Twips(size_pt * 15.0 * 0.8);
    run.max_descent = Twips(size_pt * 15.0 * 0.2);
    
    // Generate dummy glyphs for selection bounds calculation
    if (text.length() > 0) {
        Twips per_char_width(run.total_width.getValue() / text.length());
        for (size_t i = 0; i < text.length(); ++i) {
            Glyph g;
            g.x_advance = per_char_width;
            run.glyphs.push_back(g);
        }
    }
    
    return run;
  }

private:
  cairo_t *cr_;
};

int main() {
  std::cout << "Starting Lib-Pluma Demo..." << std::endl;

  // 1. Initialize Shaper & Font
  auto shaper = std::make_shared<RealCairoShaper>();
  auto font = std::make_shared<DummyFontManager>()->getFont({"Inter", 12.0f});

  // 2. Create the Agnostic Editor
  PlumaEditor editor(shaper, font);

  // 3. Page Setup: A4 with 2cm margins (~1134 Twips)
  editor.setPageSize(PageSizes::A4);
  editor.setMargins(
      PageMargins(Twips(1134), Twips(1134), Twips(1134), Twips(1134)));

  // Enable margin visibility so we can see the bounds (e.g. green box)
  editor.setMarginColor(Color(0xFF00AA00)); // Green color
  editor.showMargins();

  // 4. Load Document
  std::string text = 
      "Lib-Pluma Engine\n"
      "|OL:1:1|First ordered item (Numbers).\n"
      "|OL:1:1|Second ordered item (Numbers).\n"
      "|UL:2:square|Nested unordered item (Square).\n"
      "|UL:2:circle|Another nested unordered item (Circle).\n"
      "|OL:1:A|Third ordered item (Uppercase Letter).\n"
      "|INDENT:0.5:1.0:0.5|This paragraph is testing the new indentation system. It has a left indent of 0.5 inches, a right indent of 1.0 inch, and a first-line indent of 0.5 inches. As you can see, the text wraps earlier on the right side and is pushed inward on the left. The first line is pushed even further inward to demonstrate classic paragraph indentation behavior.\n"
      "|INDENT:0.5:0.5:-0.5|This paragraph demonstrates a hanging indent. The left indent is 0.5 inches, but the first line has a negative indent of -0.5 inches, pushing it back to the edge. The right side also has a 0.5 inch indent to keep it contained.\n"
      "|TBL:cols=2|\n"
      "|ROW|\n"
      "|CEL|\n"
      "|IMAGE:Square:/home/horacio/.gemini/antigravity/brain/9eeccdc2-3773-4138-b22d-4688b30e39af/pluma_logo_1779674789201.png| Dynamic Image Support\n"
      "|CEL|\n"
      "This is a dynamic cell next to the image.\n"
      "|ROW|\n"
      "|CEL|\n"
      "Another row in the dynamic table.\n"
      "|CEL|\n"
      "Last cell. Let's see how it formats.\n"
      "|ENDTBL|\n"
      "|COLUMNS:2|\n"
      "|DROPCAP:3|Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum condimentum velit et justo egestas, non pellentesque risus hendrerit. Phasellus a sapien nec eros mattis vehicula at at enim. Nullam dignissim sapien in magna bibendum, quis posuere lorem lobortis. Suspendisse eleifend elit odio, id iaculis neque rhoncus id.\n"
      "Donec fringilla nisi ac urna facilisis, sit amet semper ex lacinia. Cras eleifend felis nisl, in auctor metus finibus quis. Maecenas commodo augue et est convallis pretium. Pellentesque ultrices quam quis mauris tristique, et fringilla massa vestibulum. Nam facilisis urna purus, ullamcorper laoreet neque congue id. Mauris eget tellus felis. Proin nec mauris at dolor sodales cursus vitae nec erat.\n"
      "Aliquam id augue auctor, tempus libero et, gravida diam. Ut venenatis dui et cursus luctus. Integer ut tincidunt elit. Proin viverra mauris tellus. Etiam tempor dui mi, vel imperdiet risus mollis efficitur. Cras egestas accumsan interdum. Vestibulum rhoncus neque quis tincidunt fringilla.\n"
      "Phasellus condimentum scelerisque dui, nec facilisis massa interdum eget. In pellentesque sapien in dolor accumsan, in fermentum mi iaculis. Nunc sit amet enim vel orci interdum vulputate. Morbi et tellus et dui tincidunt ultrices sit amet sit amet neque. Sed tristique arcu sed massa ullamcorper rutrum.\n"
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum condimentum velit et justo egestas, non pellentesque risus hendrerit. Phasellus a sapien nec eros mattis vehicula at at enim. Nullam dignissim sapien in magna bibendum, quis posuere lorem lobortis. Suspendisse eleifend elit odio, id iaculis neque rhoncus id.\n"
      "Donec fringilla nisi ac urna facilisis, sit amet semper ex lacinia. Cras eleifend felis nisl, in auctor metus finibus quis. Maecenas commodo augue et est convallis pretium. Pellentesque ultrices quam quis mauris tristique, et fringilla massa vestibulum. Nam facilisis urna purus, ullamcorper laoreet neque congue id. Mauris eget tellus felis. Proin nec mauris at dolor sodales cursus vitae nec erat.\n"
      "Aliquam id augue auctor, tempus libero et, gravida diam. Ut venenatis dui et cursus luctus. Integer ut tincidunt elit. Proin viverra mauris tellus. Etiam tempor dui mi, vel imperdiet risus mollis efficitur. Cras egestas accumsan interdum. Vestibulum rhoncus neque quis tincidunt fringilla.\n"
      "Phasellus condimentum scelerisque dui, nec facilisis massa interdum eget. In pellentesque sapien in dolor accumsan, in fermentum mi iaculis. Nunc sit amet enim vel orci interdum vulputate. Morbi et tellus et dui tincidunt ultrices sit amet sit amet neque. Sed tristique arcu sed massa ullamcorper rutrum.\n"
      "|COLUMNS:1|\n"
      "Sed in ligula sagittis, efficitur velit sed, finibus urna. Integer in bibendum purus, vel facilisis ligula. Ut cursus, velit et tincidunt cursus, augue mi dignissim lacus, a dictum tellus purus sit amet enim. Curabitur convallis interdum metus, non tristique nisl dictum a. Ut ac felis risus.\n"
      "Vestibulum id lorem mattis, tincidunt diam at, tristique eros. Mauris feugiat congue dolor nec sollicitudin. Pellentesque aliquet, ipsum a dignissim malesuada, ligula dui ullamcorper nulla, sit amet suscipit sapien nulla sed mauris. Maecenas sollicitudin ex ut ipsum bibendum, ac rhoncus mauris mattis.\n"
      "Cras auctor dui non cursus hendrerit. Aenean pretium feugiat leo eget accumsan. Suspendisse tincidunt tristique nulla, eu scelerisque mauris interdum ut. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Nam vel finibus dolor. Sed scelerisque ligula nec neque iaculis scelerisque.\n"
      "Duis sodales ex id ante cursus gravida. Aliquam nec arcu dolor. Suspendisse pulvinar odio vel dui commodo suscipit. Vivamus elementum tellus quis quam congue, eu dapibus ex elementum. Fusce eu magna sed neque sodales cursus eget ut metus. Pellentesque fermentum tristique odio et varius.\n"
      "|IMAGE:Square:/home/horacio/.gemini/antigravity/brain/9eeccdc2-3773-4138-b22d-4688b30e39af/pluma_logo_1779674789201.png| Vestibulum id lorem mattis, tincidunt diam at, tristique eros. Mauris feugiat congue dolor nec sollicitudin. Pellentesque aliquet, ipsum a dignissim malesuada, ligula dui ullamcorper nulla, sit amet suscipit sapien nulla sed mauris.\n"
      "|IMAGE:Tight:/home/horacio/.gemini/antigravity/brain/9eeccdc2-3773-4138-b22d-4688b30e39af/pluma_logo_1779674789201.png| Cras auctor dui non cursus hendrerit. Aenean pretium feugiat leo eget accumsan. Suspendisse tincidunt tristique nulla, eu scelerisque mauris interdum ut.\n"
      "Water is H2O, and Sulfuric Acid is H2SO4. Einstein said E=mc2.";
  
  editor.loadText(text);

  // 5. Apply Rich Text Styles
  // Title: Bold, Centered, Underlined
  editor.applyStyle(0, 16, PropertyId::FontWeight, uint16_t(700));
  editor.applyStyle(0, 16, PropertyId::TextAlignment, TextAlign::Center);
  editor.applyStyle(0, 16, PropertyId::Decoration, TextDecoration::Underline);
  editor.applyStyle(0, 16, PropertyId::ParagraphSpacingAfter, 12.0f); // 12pt space after title

  uint32_t pos = 17;
  
  // Paragraph 1: Gray, Left, 1.5x spacing, Larger Font!
  editor.applyStyle(pos, 287, PropertyId::TextColor, Color(0xFF555555));
  editor.applyStyle(pos, 287, PropertyId::TextAlignment, TextAlign::Left);
  editor.applyStyle(pos, 287, PropertyId::LineSpacing, 1.5f);
  editor.applyStyle(pos, 287, PropertyId::ParagraphSpacingAfter, 20.0f);
  editor.applyStyle(pos, 287, PropertyId::FontSize, 18.0f); // 18pt font
  pos += 287; // 304

  // Paragraph 2: Red, Centered, 1.2x spacing
  editor.applyStyle(pos, 388, PropertyId::TextColor, Color(0xFFAA0000));
  editor.applyStyle(pos, 388, PropertyId::TextAlignment, TextAlign::Center);
  editor.applyStyle(pos, 388, PropertyId::LineSpacing, 1.2f);
  editor.applyStyle(pos, 388, PropertyId::ParagraphSpacingAfter, 20.0f);
  pos += 388; // 692

  // Paragraph 3: Blue, Right, double spacing
  editor.applyStyle(pos, 255, PropertyId::TextColor, Color(0xFF0000AA));
  editor.applyStyle(pos, 255, PropertyId::TextAlignment, TextAlign::Right);
  editor.applyStyle(pos, 255, PropertyId::LineSpacing, 2.0f);
  editor.applyStyle(pos, 255, PropertyId::ParagraphSpacingAfter, 15.0f);
  pos += 255; // 947

  // Paragraph 4: Green, Justified (simulated as left if unsupported), tight spacing
  editor.applyStyle(pos, 281, PropertyId::TextColor, Color(0xFF008800));
  editor.applyStyle(pos, 281, PropertyId::TextAlignment, TextAlign::Justify);
  editor.applyStyle(pos, 281, PropertyId::LineSpacing, 0.9f);
  editor.applyStyle(pos, 281, PropertyId::ParagraphSpacingAfter, 30.0f);
  pos += 281;

  // Paragraph 5: Orange, Center, normal spacing, HUGE font
  editor.applyStyle(pos, 274, PropertyId::TextColor, Color(0xFFDD6600));
  editor.applyStyle(pos, 274, PropertyId::TextAlignment, TextAlign::Center);
  editor.applyStyle(pos, 274, PropertyId::ParagraphSpacingAfter, 10.0f);
  editor.applyStyle(pos, 274, PropertyId::FontSize, 28.0f); // 28pt font
  pos += 274;

  // Paragraph 6: Purple, Left, massive spacing before/after to force pagination
  editor.applyStyle(pos, 290, PropertyId::TextColor, Color(0xFF880088));
  editor.applyStyle(pos, 290, PropertyId::LineSpacing, 1.3f);
  editor.applyStyle(pos, 290, PropertyId::ParagraphSpacingBefore, 50.0f);
  editor.applyStyle(pos, 290, PropertyId::ParagraphSpacingAfter, 40.0f);
  pos += 290;

  // Paragraph 7: Teal, Right, 1.8x spacing
  editor.applyStyle(pos, 291, PropertyId::TextColor, Color(0xFF008888));
  editor.applyStyle(pos, 291, PropertyId::TextAlignment, TextAlign::Right);
  editor.applyStyle(pos, 291, PropertyId::LineSpacing, 1.8f);
  editor.applyStyle(pos, 291, PropertyId::ParagraphSpacingAfter, 25.0f);
  pos += 291;

  // Paragraph 8: Black, Left, Underlined
  editor.applyStyle(pos, 287, PropertyId::TextColor, Color(0xFF000000));
  editor.applyStyle(pos, 287, PropertyId::Decoration, TextDecoration::Underline);
  
  pos = text.find("Water is H2O");

  // Paragraph 9: Formulas
  // "Water is H2O, and Sulfuric Acid is H2SO4. Einstein said E=mc2."
  editor.applyStyle(pos, 62, PropertyId::FontSize, 20.0f); // Make formula text larger
  editor.applyStyle(pos, 62, PropertyId::TextColor, Color(0xFF222222));
  
  // Subscript for '2' in H2O
  editor.applyStyle(pos + 10, 1, PropertyId::VerticalAlignment, VerticalAlign::Subscript);
  // Subscript for '2' in H2SO4
  editor.applyStyle(pos + 36, 1, PropertyId::VerticalAlignment, VerticalAlign::Subscript);
  // Subscript for '4' in H2SO4
  editor.applyStyle(pos + 39, 1, PropertyId::VerticalAlignment, VerticalAlign::Subscript);
  // Superscript for '2' in E=mc2
  editor.applyStyle(pos + 60, 1, PropertyId::VerticalAlignment, VerticalAlign::Superscript);
  
  editor.setCursorStateCallback([](const pluma::CursorState& state) {
      const char* type_str = "Text";
      if (state.object_type == pluma::CursorObjectType::Image) type_str = "Image";
      else if (state.object_type == pluma::CursorObjectType::Table) type_str = "Table";
      else if (state.object_type == pluma::CursorObjectType::TableRow) type_str = "TableRow";
      else if (state.object_type == pluma::CursorObjectType::TableColumn) type_str = "TableColumn";
      else if (state.object_type == pluma::CursorObjectType::TableCell) type_str = "TableCell";

      std::cout << "[Cursor Callback] Pos: " << state.logical_offset 
                << ", Type: " << type_str << std::endl;
  });

  // Also configure image sizes so they render nicely (instead of 200x200 default)
  // Our generated images are likely 1024x1024, let's scale them to 150pt
  for (uint32_t p = 0; p < text.length(); ++p) {
      if (text.substr(p, 7) == "|IMAGE:") {
          editor.applyStyle(p, 1, PropertyId::ImageWidth, 150.0f);
          editor.applyStyle(p, 1, PropertyId::ImageHeight, 150.0f);
      }
  }
  
  // Also, let's select a column in the table!
  size_t tbl_pos = text.find("|TBL:cols=2|");
  if (tbl_pos != std::string::npos) {
      // editor.selectTableColumn(tbl_pos, 0); // We comment this out to see the text selection instead
  }
  
  // Select some text
  size_t lorem_pos = text.find("Lorem ipsum");
  if (lorem_pos != std::string::npos) {
      editor.setSelection(lorem_pos, lorem_pos + 65); // Select "Lorem ipsum dolor sit amet, consectetur adipiscing elit."
  }

  // 6. Render to a Cairo Surface
  // Let's make it 4 pages tall to see extensive pagination.
  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 794, 1123 * 4);
  cairo_t *cr = cairo_create(surface);

  // Tell the editor's virtualization engine that our "window" is exactly 4 pages tall,
  // otherwise it will cull anything below its default 10000-twip height.
  editor.setViewport(Twips(794 * 15), Twips(1123 * 4 * 15));

  // Fill white background for the PNG
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);

  CairoRenderer renderer(cr);
  editor.render(renderer); // Instant virtualized render!

  // 7. Save output to PNG
  const char *filename = "pluma_render_output.png";
  cairo_status_t status = cairo_surface_write_to_png(surface, filename);

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  if (status == CAIRO_STATUS_SUCCESS) {
    std::cout << "Successfully rendered document to: " << filename << std::endl;
  } else {
    std::cerr << "Failed to render document. Cairo status: "
              << cairo_status_to_string(status) << std::endl;
    return 1;
  }

  // 8. Export to PDF
  pluma::plugins::PdfExporter pdf_exporter;
  if (pdf_exporter.exportToFile("pluma_render_output.pdf", editor)) {
    std::cout << "Successfully exported document to: pluma_render_output.pdf" << std::endl;
  } else {
    std::cerr << "Failed to export document to PDF." << std::endl;
    return 1;
  }

  return 0;
}
