#include <pluma/Services/SpellCheckAnalyzer.hpp>
#include <cctype>

namespace pluma {

SpellCheckAnalyzer::SpellCheckAnalyzer(std::shared_ptr<SpellCheckerService> spell_checker, std::string default_lang, ErrorListCallback on_complete)
    : spell_checker_(std::move(spell_checker)), default_lang_(std::move(default_lang)), on_complete_(std::move(on_complete)) {
}

void SpellCheckAnalyzer::analyze(std::shared_ptr<DocumentSnapshot> snapshot) {
    if (!snapshot || !spell_checker_) return;

    std::string text = snapshot->getText();
    
    // For MVP we assume a default document language if not specified per span
    // In a complete implementation we would need to check FormatRegistry for PropertyId::Language per span.
    std::string current_lang = default_lang_; 
    
    std::string current_word;
    uint32_t word_start = 0;

    std::vector<std::pair<uint32_t, uint32_t>> errors;

    for (uint32_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (std::isalpha(static_cast<unsigned char>(c))) {
            if (current_word.empty()) {
                word_start = i;
            }
            current_word += c;
        } else {
            if (!current_word.empty()) {
                if (!spell_checker_->checkWord(current_word, current_lang)) {
                    errors.push_back({word_start, current_word.length()});
                }
                current_word.clear();
            }
        }
    }

    if (!current_word.empty()) {
        if (!spell_checker_->checkWord(current_word, current_lang)) {
            errors.push_back({word_start, current_word.length()});
        }
    }
    
    if (on_complete_) {
        on_complete_(errors);
    }
}

} // namespace pluma
