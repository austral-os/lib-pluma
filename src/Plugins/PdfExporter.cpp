#include <pluma/Plugins/PdfExporter.hpp>
#include <pluma/Render/CairoRenderer.hpp>
#include <cairo-pdf.h>

namespace pluma {
namespace plugins {

std::string PdfExporter::exportDoc(std::shared_ptr<DocumentSnapshot> /* snapshot */) {
    // We cannot export a PDF to a raw string cleanly here without geometry.
    // Callers should use exportToFile.
    return "";
}

bool PdfExporter::exportToFile(const std::string& filename, PlumaEditor& editor) {
    // A4 size in Twips: 11905 x 16838
    Twips a4_width(11905);
    Twips a4_height(16838);

    // Cairo PDF surface uses Points (72 DPI). 1 Point = 20 Twips.
    double width_pt = a4_width.getValue() / 20.0;
    double height_pt = a4_height.getValue() / 20.0;

    cairo_surface_t* surface = cairo_pdf_surface_create(filename.c_str(), width_pt, height_pt);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surface);
        return false;
    }

    cairo_t* cr = cairo_create(surface);

    // CairoRenderer outputs at 96 DPI (divides Twips by 15).
    // The PDF surface expects 72 DPI (divides Twips by 20).
    // We scale the context by 15/20 = 0.75 so that CairoRenderer's 96 DPI output 
    // correctly maps to the 72 DPI PDF coordinates.
    cairo_scale(cr, 0.75, 0.75);

    CairoRenderer renderer(cr);
    
    Color old_bg  = editor.getPageBackgroundColor();
    Color old_txt = editor.getDefaultTextColor();
    
    // 1. First do layout-triggering changes so updateLayout() doesn't re-sync theme colors
    editor.setPageGap(Twips(0));
    editor.hideMargins();
    editor.setCaretVisible(false);
    editor.setPrinting(true);
    
    // 2. NOW set print colors — these must come AFTER any call that triggers updateLayout()
    //    because the PlumaView theme sync runs inside updateLayout's notification chain
    editor.setWorkspaceBackgroundColor(Color(0xFFFFFFFF));
    editor.setPageBackgroundColor(Color(0xFFFFFFFF));
    editor.setDefaultTextColor(Color(0xFF000000));
    
    size_t page_count = editor.getPageCount();
    if (page_count == 0) page_count = 1;

    for (size_t i = 0; i < page_count; ++i) {
        editor.setViewport(a4_width, a4_height);
        editor.setScroll(Twips(0), Twips(a4_height.getValue() * i));
        
        // 3. Guarantee white page background via Cairo directly, in case the editor's
        //    workspace_bg_color_ still got overridden by the theme sync path
        double page_w = a4_width.getValue() / 15.0;   // Twips -> 96DPI pixels
        double page_h = a4_height.getValue() / 15.0;
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);      // pure white
        cairo_rectangle(cr, 0.0, 0.0, page_w, page_h);
        cairo_fill(cr);
        
        editor.render(renderer);
        
        cairo_show_page(cr); // Emit the page to the PDF
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    // Restore editor state
    editor.setWorkspaceBackgroundColor(Color(0xFFE0E0E0));
    editor.setPageBackgroundColor(old_bg);
    editor.setDefaultTextColor(old_txt);
    editor.setPageGap(Twips(400));
    editor.showMargins();
    editor.setCaretVisible(true);
    editor.setPrinting(false);
    editor.setScroll(Twips(0), Twips(0));
    editor.setViewport(Twips(794 * 15), Twips(1123 * 3 * 15));

    return true;
}

} // namespace plugins
} // namespace pluma
