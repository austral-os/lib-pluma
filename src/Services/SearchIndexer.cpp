#include <pluma/Services/SearchIndexer.hpp>
#include <sstream>
#include <cctype>

namespace pluma {

void SearchIndexer::analyze(std::shared_ptr<DocumentSnapshot> snapshot, pluma::FormatRegistry styles) {
    (void)styles;
    if (!snapshot) return;

    std::string text = snapshot->getText();
    
    std::unordered_map<std::string, int> local_counts;
    
    // Very simple tokenizer for indexing
    std::string current_word;
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            current_word += std::tolower(static_cast<unsigned char>(c));
        } else if (!current_word.empty()) {
            local_counts[current_word]++;
            current_word.clear();
        }
    }
    if (!current_word.empty()) {
        local_counts[current_word]++;
    }

    // Safely update the shared state
    std::lock_guard<std::mutex> lock(mutex_);
    word_counts_ = std::move(local_counts);
}

int SearchIndexer::getWordFrequency(const std::string& word) {
    std::string lower_word;
    for (char c : word) {
        lower_word += std::tolower(static_cast<unsigned char>(c));
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = word_counts_.find(lower_word);
    if (it != word_counts_.end()) {
        return it->second;
    }
    return 0;
}

} // namespace pluma
