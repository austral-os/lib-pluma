/**
 * @file ShaperCache.hpp
 * @brief Caches typographic metrics to prevent redundant text shaping.
 */
#pragma once
#include <pluma/Typography/ITextShaper.hpp>
#include <unordered_map>
#include <mutex>
#include <string>

namespace pluma {
namespace optimization {

/**
 * @class ShaperCache
 * @brief Decorator for ITextShaper that caches ShapedTextRun results.
 */
class ShaperCache : public ITextShaper {
public:
    explicit ShaperCache(std::shared_ptr<ITextShaper> fallback) : fallback_(std::move(fallback)) {}

    ShapedTextRun shapeText(std::string_view text, const std::shared_ptr<IFont>& font) override {
        // Simple cache using the text string as a key.
        // In a real implementation, the key would include font properties.
        std::string key(text);
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second;
        }
        
        auto run = fallback_->shapeText(text, font);
        cache_[key] = run;
        return run;
    }

private:
    std::shared_ptr<ITextShaper> fallback_;
    std::unordered_map<std::string, ShapedTextRun> cache_;
    std::mutex mutex_;
};

} // namespace optimization
} // namespace pluma
