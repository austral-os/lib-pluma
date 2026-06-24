#include <pluma/Services/SpellCheckerService.hpp>
#include <nuspell/dictionary.hxx>
#include <nuspell/finder.hxx>

namespace pluma {

SpellCheckerService::SpellCheckerService() = default;

SpellCheckerService::~SpellCheckerService() = default;

bool SpellCheckerService::loadDictionary(const std::string& langCode, const std::string& affPath, const std::string& /*dicPath*/) {
    try {
        auto dict = std::make_unique<nuspell::Dictionary>();
        dict->load_aff_dic(affPath);
        std::lock_guard<std::mutex> lock(mutex_);
        dictionaries_[langCode] = std::move(dict);
        return true;
    } catch (...) {
        return false;
    }
}

void SpellCheckerService::registerDictionary(const std::string& langCode, const std::string& affPath, const std::string& dicPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_dictionaries_[langCode] = {affPath, dicPath};
}

bool SpellCheckerService::ensureDictionaryLoaded(const std::string& langCode) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (dictionaries_.find(langCode) != dictionaries_.end()) {
        return true;
    }
    auto it = pending_dictionaries_.find(langCode);
    if (it != pending_dictionaries_.end()) {
        try {
            auto dict = std::make_unique<nuspell::Dictionary>();
            dict->load_aff_dic(it->second.first);
            dictionaries_[langCode] = std::move(dict);
            return true;
        } catch (...) {
            return false;
        }
    }
    return false;
}

void SpellCheckerService::ignoreWord(const std::string& word) {
    std::lock_guard<std::mutex> lock(mutex_);
    ignored_words_.insert(word);
}

bool SpellCheckerService::checkWord(std::string_view word, const std::string& langCode) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string w(word);
    if (ignored_words_.find(w) != ignored_words_.end()) {
        return true;
    }
    auto it = dictionaries_.find(langCode);
    if (it != dictionaries_.end()) {
        return it->second->spell(w);
    }
    // If language is not loaded, we consider it "correct" to not show false positives
    return true;
}

std::vector<std::string> SpellCheckerService::getSuggestions(std::string_view word, const std::string& langCode) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = dictionaries_.find(langCode);
    if (it != dictionaries_.end()) {
        std::string w(word);
        std::vector<std::string> results;
        it->second->suggest(w, results);
        return results;
    }
    return {};
}

bool SpellCheckerService::hasLanguage(const std::string& langCode) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return dictionaries_.find(langCode) != dictionaries_.end() || pending_dictionaries_.find(langCode) != pending_dictionaries_.end();
}

} // namespace pluma
