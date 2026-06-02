import re
import os

docs_dir = "/home/horacio/HdrDevs/lib-pluma/docs"
api_ref_path = os.path.join(docs_dir, "API_REFERENCE.md")

with open(api_ref_path, "r") as f:
    content = f.read()

# Add a stylish header
header = """<div align="center">
  <img src="https://via.placeholder.com/150?text=Pluma+Logo" alt="Lib-Pluma Logo" />
  <h1>🪶 Lib-Pluma Core API Reference</h1>
  <p><b>A High-Performance C++ Rich Text Layout and Composition Engine</b></p>
</div>

Welcome to the **lib-pluma API Reference**. This documentation covers the core components needed to integrate the Pluma engine into your desktop application.

---

## 🚀 Quick Start Example

Before diving into the detailed class documentation, here is a complete example of how to initialize the `PlumaEditor` and create a rich text document:

```cpp
#include <pluma/PlumaEditor.hpp>
#include <pluma/Typography/DummyTypography.hpp>

int main() {
    // 1. Initialize text shaper and default font
    auto shaper = std::make_shared<pluma::DummyTextShaper>();
    auto font = std::make_shared<pluma::DummyFont>(pluma::FontDescriptor{"Inter", 12.0f, pluma::FontWeight::Normal});

    // 2. Create the editor instance
    pluma::PlumaEditor editor(shaper, font);

    // 3. Set page geometry (A4 size, 1-inch margins)
    editor.setPageSize(pluma::PageSizes::A4);
    editor.setMargins(pluma::PageMargins(1440, 1440, 1440, 1440));

    // 4. Load content with advanced components (Tables and Variables)
    editor.setHeader("Confidential - Page {PAGE} of {NUMPAGES}");
    editor.loadText(
        "Welcome to Pluma!\\n"
        "|TBL:cols=2|\\n"
        "|ROW|\\n"
        "|CEL|\\nCell 1 Content\\n"
        "|CEL|\\nCell 2 Content\\n"
        "|ENDTBL|"
    );

    // 5. Apply rich formatting (bold text)
    editor.applyStyle(0, 7, pluma::PropertyId::FontWeight, (uint16_t)pluma::FontWeight::Bold);
    
    // 6. Trigger layout update
    editor.updateLayout();

    return 0;
}
```

---

## 🏛 Architecture Overview: Agnostic Rendering & Text Shaping

`lib-pluma` is designed as a **platform-agnostic** document composition engine. Its primary responsibility is document structuring, PieceTable management, pagination, and layout math. **It does not perform pixel rasterization or font glyph shaping internally.**

Instead, `lib-pluma` uses **Inversion of Control (IoC)**, requiring the embedding application to inject two platform-specific dependencies:

1. **`ITextShaper`**: Provided by the application to measure text extents (e.g., using HarfBuzz, CoreText, or DirectWrite). It converts a string into a `ShapedTextRun` containing individual `Glyph` metrics.
2. **`IRenderer`**: Provided by the application to execute primitive drawing commands (e.g., using Cairo, Skia, or Qt). It takes the computed `DisplayList` and paints rectangles, images, and text.

This decoupling ensures `lib-pluma` remains blazing fast and portable across all operating systems without hard dependencies on UI frameworks.

---

"""

# Write back index
with open(api_ref_path, "w") as f:
    f.write(header + content.replace("# Lib-Pluma API Reference\n\nWelcome to the **lib-pluma API Reference**. Below you will find the components organized by module.\n\n", "## 📚 Modules\n\n"))

# Add PlumaEditor example to the actual class file
editor_md_path = os.path.join(docs_dir, "api/Core/pluma_PlumaEditor.md")
if os.path.exists(editor_md_path):
    with open(editor_md_path, "r") as f:
        editor_content = f.read()

    editor_example = """

### 💡 Example Usage: Interacting with Tables

```cpp
// 1. Mouse down near a column border triggers resize mode
editor.onMouseDown(x, y, pluma::MouseButton::Left, pluma::ModifierFlags::None);

// 2. Moving the mouse updates the column width in real time
editor.onMouseMove(new_x, y, pluma::ModifierFlags::None);

// 3. Mouse up commits the layout
editor.onMouseUp(new_x, y, pluma::MouseButton::Left, pluma::ModifierFlags::None);
```
"""
    editor_content = re.sub(r'(## Public Methods)', editor_example + r'\1', editor_content)
    
    with open(editor_md_path, "w") as f:
        f.write(editor_content)

print("Documentation improved successfully!")
