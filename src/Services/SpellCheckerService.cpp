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
        dictionaries_[langCode] = std::move(dict);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool SpellCheckerService::checkWord(std::string_view word, const std::string& langCode) const {
    auto it = dictionaries_.find(langCode);
    if (it == dictionaries_.end()) {
        return true; // If we don't have the dictionary, assume correct to avoid false positives
    }
    
    return it->second->spell(std::string(word));
}

std::vector<std::string> SpellCheckerService::getSuggestions(std::string_view word, const std::string& langCode) const {
    auto it = dictionaries_.find(langCode);
    if (it == dictionaries_.end()) {
        return {};
    }
    
    std::vector<std::string> sugs;
    it->second->suggest(std::string(word), sugs);
    return sugs;
}

bool SpellCheckerService::hasLanguage(const std::string& langCode) const {
    return dictionaries_.find(langCode) != dictionaries_.end();
}

} // namespace pluma
