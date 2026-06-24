#include <pluma/Services/SpellCheckAnalyzer.hpp>
#include <cctype>

namespace pluma {

SpellCheckAnalyzer::SpellCheckAnalyzer(std::shared_ptr<SpellCheckerService> spell_checker, std::string default_lang, ErrorListCallback on_complete)
    : spell_checker_(std::move(spell_checker)), default_lang_(std::move(default_lang)), on_complete_(std::move(on_complete)) {
}

void SpellCheckAnalyzer::analyze(std::shared_ptr<DocumentSnapshot> snapshot, pluma::FormatRegistry styles) {
    if (!snapshot || !spell_checker_) return;

    std::string text = snapshot->getText();
    
    std::string current_word;
    uint32_t word_start = 0;

    std::vector<std::pair<uint32_t, uint32_t>> errors;

    auto is_word_char = [](const std::string& str, size_t i) -> bool {
        unsigned char c = str[i];
        if (std::isalpha(c)) return true;
        // Latin-1 Supplement letters (á, é, í, ó, ú, ñ, ü, etc.) are 0xC3 followed by 0x80-0xBF
        if (c == 0xC3 && i + 1 < str.length()) {
            unsigned char c2 = str[i+1];
            // Skip × (0xC3 0x97) and ÷ (0xC3 0xB7)
            if (c2 == 0x97 || c2 == 0xB7) return false;
            return true;
        }
        return false;
    };

    auto get_char_len = [](unsigned char c) -> size_t {
        if ((c & 0x80) == 0) return 1;
        if ((c & 0xE0) == 0xC0) return 2;
        if ((c & 0xF0) == 0xE0) return 3;
        if ((c & 0xF8) == 0xF0) return 4;
        return 1;
    };

    for (uint32_t i = 0; i < text.size();) {
        if (is_word_char(text, i)) {
            if (current_word.empty()) {
                word_start = i;
            }
            size_t char_len = get_char_len(text[i]);
            current_word.append(text, i, char_len);
            i += char_len;
        } else {
            if (!current_word.empty()) {
                std::string current_lang = default_lang_;
                auto style_opt = styles.getStyleAt(word_start).get(pluma::PropertyId::Language);
                if (style_opt) {
                    current_lang = std::get<std::string>(*style_opt);
                }
                
                spell_checker_->ensureDictionaryLoaded(current_lang);
                if (!spell_checker_->checkWord(current_word, current_lang)) {
                    errors.push_back({word_start, current_word.length()});
                }
                current_word.clear();
            }
            i += get_char_len(text[i]);
        }
    }

    if (!current_word.empty()) {
        std::string current_lang = default_lang_;
        auto style_opt = styles.getStyleAt(word_start).get(pluma::PropertyId::Language);
        if (style_opt) {
            current_lang = std::get<std::string>(*style_opt);
        }
        
        spell_checker_->ensureDictionaryLoaded(current_lang);
        if (!spell_checker_->checkWord(current_word, current_lang)) {
            errors.push_back({word_start, current_word.length()});
        }
    }
    
    if (on_complete_) {
        on_complete_(errors);
    }
}

} // namespace pluma
