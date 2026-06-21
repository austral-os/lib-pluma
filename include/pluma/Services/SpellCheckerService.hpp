#pragma once

#include <string>
#include <vector>
#include <memory>
#include <string_view>
#include <unordered_map>

#include <nuspell/dictionary.hxx>
#include <mutex>

namespace pluma {

class SpellCheckerService {
public:
    SpellCheckerService();
    ~SpellCheckerService();

    /**
     * @brief Load a dictionary for a specific language code.
     * @param langCode e.g., "en-US", "es-ES"
     * @param affPath Path to the .aff file
     * @param dicPath Path to the .dic file
     * @return true if successfully loaded, false otherwise
     */
    bool loadDictionary(const std::string& langCode, const std::string& affPath, const std::string& dicPath);

    /**
     * @brief Check if a word is spelled correctly in the given language.
     * @param word The word to check
     * @param langCode The language code (must have been loaded)
     * @return true if correct, false if incorrect or language not found
     */
    bool checkWord(std::string_view word, const std::string& langCode) const;

    /**
     * @brief Get spelling suggestions for an incorrect word.
     * @param word The misspelled word
     * @param langCode The language code
     * @return A list of suggested corrections
     */
    std::vector<std::string> getSuggestions(std::string_view word, const std::string& langCode) const;

    /**
     * @brief Check if a dictionary for a language code is loaded.
     */
    bool hasLanguage(const std::string& langCode) const;

    /**
     * @brief Register the paths for a dictionary without loading it.
     */
    void registerDictionary(const std::string& langCode, const std::string& affPath, const std::string& dicPath);

    /**
     * @brief Ensure the dictionary for the language is loaded.
     */
    bool ensureDictionaryLoaded(const std::string& langCode);

private:
    std::unordered_map<std::string, std::unique_ptr<nuspell::Dictionary>> dictionaries_;
    std::unordered_map<std::string, std::pair<std::string, std::string>> pending_dictionaries_;
    mutable std::mutex mutex_;
};

} // namespace pluma
