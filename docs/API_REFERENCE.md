<div align="center">
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
        "Welcome to Pluma!\n"
        "|TBL:cols=2|\n"
        "|ROW|\n"
        "|CEL|\nCell 1 Content\n"
        "|CEL|\nCell 2 Content\n"
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

## 📚 Modules

## A11y

- [pluma::a11y::A11yNode](api/A11y/pluma_a11y_A11yNode.md)
- [pluma::a11y::A11yTreeBuilder](api/A11y/pluma_a11y_A11yTreeBuilder.md)

## Core

- [pluma::Command](api/Core/pluma_Command.md)
- [pluma::CursorState](api/Core/pluma_CursorState.md)
- [pluma::DeleteTextCommand](api/Core/pluma_DeleteTextCommand.md)
- [pluma::DocumentSnapshot](api/Core/pluma_DocumentSnapshot.md)
- [pluma::InsertTextCommand](api/Core/pluma_InsertTextCommand.md)
- [pluma::NodeId](api/Core/pluma_NodeId.md)
- [pluma::Piece](api/Core/pluma_Piece.md)
- [pluma::PieceTable](api/Core/pluma_PieceTable.md)
- [pluma::PieceTable::Position](api/Core/pluma_PieceTable_Position.md)
- [pluma::PlumaEditor](api/Core/pluma_PlumaEditor.md)
- [pluma::Point](api/Core/pluma_Point.md)
- [pluma::Rect](api/Core/pluma_Rect.md)
- [pluma::RevisionId](api/Core/pluma_RevisionId.md)
- [pluma::Size](api/Core/pluma_Size.md)
- [pluma::TableSelection](api/Core/pluma_TableSelection.md)
- [pluma::TextSlice](api/Core/pluma_TextSlice.md)
- [pluma::Transaction](api/Core/pluma_Transaction.md)
- [pluma::Twips](api/Core/pluma_Twips.md)
- [pluma::UndoManager](api/Core/pluma_UndoManager.md)
- [pluma::UndoManager::HistoryEntry](api/Core/pluma_UndoManager_HistoryEntry.md)
- [pluma::plugins::PropertyValueToJson](api/Core/pluma_plugins_PropertyValueToJson.md)

## DOM

- [pluma::DOMManager](api/DOM/pluma_DOMManager.md)
- [pluma::DOMNode](api/DOM/pluma_DOMNode.md)
- [pluma::DocumentNode](api/DOM/pluma_DocumentNode.md)
- [pluma::ParagraphNode](api/DOM/pluma_ParagraphNode.md)
- [pluma::RunNode](api/DOM/pluma_RunNode.md)
- [pluma::TableCellNode](api/DOM/pluma_TableCellNode.md)
- [pluma::TableNode](api/DOM/pluma_TableNode.md)
- [pluma::TableRowNode](api/DOM/pluma_TableRowNode.md)

## Diagnostics

- [pluma::diagnostics::Dumper](api/Diagnostics/pluma_diagnostics_Dumper.md)
- [pluma::diagnostics::Profiler](api/Diagnostics/pluma_diagnostics_Profiler.md)
- [pluma::diagnostics::ScopedTimer](api/Diagnostics/pluma_diagnostics_ScopedTimer.md)

## Editor

- [pluma::CaretResolver](api/Editor/pluma_CaretResolver.md)
- [pluma::SelectionRange](api/Editor/pluma_SelectionRange.md)

## Input

- [pluma::InputRouter](api/Input/pluma_InputRouter.md)
- [pluma::InputRouter::ShortcutHash](api/Input/pluma_InputRouter_ShortcutHash.md)
- [pluma::InputRouter::ShortcutKey](api/Input/pluma_InputRouter_ShortcutKey.md)

## Layout

- [pluma::BlockBox](api/Layout/pluma_BlockBox.md)
- [pluma::DirtyRegion](api/Layout/pluma_DirtyRegion.md)
- [pluma::ImageBox](api/Layout/pluma_ImageBox.md)
- [pluma::InvalidationTracker](api/Layout/pluma_InvalidationTracker.md)
- [pluma::LayoutBox](api/Layout/pluma_LayoutBox.md)
- [pluma::LayoutEngine](api/Layout/pluma_LayoutEngine.md)
- [pluma::LayoutScheduler](api/Layout/pluma_LayoutScheduler.md)
- [pluma::LayoutTask](api/Layout/pluma_LayoutTask.md)
- [pluma::LineBox](api/Layout/pluma_LineBox.md)
- [pluma::PageBox](api/Layout/pluma_PageBox.md)
- [pluma::PageMargins](api/Layout/pluma_PageMargins.md)
- [pluma::PageSize](api/Layout/pluma_PageSize.md)
- [pluma::RunBox](api/Layout/pluma_RunBox.md)
- [pluma::TableBox](api/Layout/pluma_TableBox.md)
- [pluma::TableCellBox](api/Layout/pluma_TableCellBox.md)
- [pluma::TableRowBox](api/Layout/pluma_TableRowBox.md)

## Optimization

- [pluma::optimization::ObjectPool](api/Optimization/pluma_optimization_ObjectPool.md)
- [pluma::optimization::ShaperCache](api/Optimization/pluma_optimization_ShaperCache.md)

## Plugins

- [pluma::plugins::IExporter](api/Plugins/pluma_plugins_IExporter.md)
- [pluma::plugins::IImporter](api/Plugins/pluma_plugins_IImporter.md)
- [pluma::plugins::MarkdownExporter](api/Plugins/pluma_plugins_MarkdownExporter.md)
- [pluma::plugins::MarkdownImporter](api/Plugins/pluma_plugins_MarkdownImporter.md)
- [pluma::plugins::PdfExporter](api/Plugins/pluma_plugins_PdfExporter.md)
- [pluma::plugins::PlumaArchiveExporter](api/Plugins/pluma_plugins_PlumaArchiveExporter.md)
- [pluma::plugins::PlumaArchiveImporter](api/Plugins/pluma_plugins_PlumaArchiveImporter.md)

## Render

- [pluma::CairoRenderer](api/Render/pluma_CairoRenderer.md)
- [pluma::DisplayCommand](api/Render/pluma_DisplayCommand.md)
- [pluma::DisplayList](api/Render/pluma_DisplayList.md)
- [pluma::DrawGlyphRunCommand](api/Render/pluma_DrawGlyphRunCommand.md)
- [pluma::DrawImageCommand](api/Render/pluma_DrawImageCommand.md)
- [pluma::DummyRenderer](api/Render/pluma_DummyRenderer.md)
- [pluma::FillRectCommand](api/Render/pluma_FillRectCommand.md)
- [pluma::IRenderer](api/Render/pluma_IRenderer.md)
- [pluma::PopClipCommand](api/Render/pluma_PopClipCommand.md)
- [pluma::PushClipCommand](api/Render/pluma_PushClipCommand.md)

## Services

- [pluma::IDocumentService](api/Services/pluma_IDocumentService.md)
- [pluma::SearchIndexer](api/Services/pluma_SearchIndexer.md)
- [pluma::ServiceManager](api/Services/pluma_ServiceManager.md)

## Style

- [pluma::FontDescriptor](api/Style/pluma_FontDescriptor.md)
- [pluma::FormatRegistry](api/Style/pluma_FormatRegistry.md)
- [pluma::PropertyBag](api/Style/pluma_PropertyBag.md)
- [pluma::StyleResolver](api/Style/pluma_StyleResolver.md)
- [pluma::StyleSpan](api/Style/pluma_StyleSpan.md)

## Typography

- [pluma::DummyFont](api/Typography/pluma_DummyFont.md)
- [pluma::DummyFontManager](api/Typography/pluma_DummyFontManager.md)
- [pluma::DummyTextShaper](api/Typography/pluma_DummyTextShaper.md)
- [pluma::Glyph](api/Typography/pluma_Glyph.md)
- [pluma::IFont](api/Typography/pluma_IFont.md)
- [pluma::IFontManager](api/Typography/pluma_IFontManager.md)
- [pluma::ITextShaper](api/Typography/pluma_ITextShaper.md)
- [pluma::ShapedTextRun](api/Typography/pluma_ShapedTextRun.md)

