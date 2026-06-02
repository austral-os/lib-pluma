/**
 * @file SearchIndexer.hpp
 * @brief A document service that indexes word frequencies.
 */
#pragma once
#include <pluma/Services/IDocumentService.hpp>
#include <unordered_map>
#include <mutex>
#include <string>

namespace pluma {

/**
 * @class SearchIndexer
 * @brief Simple reverse-index service for fast document searching.
 */
class SearchIndexer : public IDocumentService {
public:
    void analyze(std::shared_ptr<DocumentSnapshot> snapshot) override;
    std::string getName() const override { return "SearchIndexer"; }
    
    /**
     * @brief Retrieves the occurrence frequency of a given word.
     */
    int getWordFrequency(const std::string& word);
    
private:
    std::mutex mutex_;
    std::unordered_map<std::string, int> word_counts_;
};

} // namespace pluma
