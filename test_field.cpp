#include <iostream>
#include <memory>
#include <pluma/PlumaEditor.hpp>
#include <pluma/Typography/DummyTypography.hpp>

using namespace pluma;

int main() {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font = std::make_shared<DummyFont>(FontDescriptor{"sans", 12, false, false});
    PlumaEditor editor(shaper, font);
    
    std::cout << "Inserting text...\n";
    editor.insertTextAtCursor("|FIELD:DATE|");
    std::cout << "Text inserted!\n";
    
    std::cout << "Simulating backspace...\n";
    editor.onEditorAction(EditorAction::DeleteBackward, "", ModifierFlags::None);
    std::cout << "Backspace simulated!\n";
    
    return 0;
}
