#pragma once

#include <pluma/Services/IDocumentService.hpp>
#include <pluma/Services/SpellCheckerService.hpp>
#include <functional>

namespace pluma {

/**
 * @class SpellCheckAnalyzer
 * @brief Background service that runs the SpellCheckerService over the document snapshot.
 */
class SpellCheckAnalyzer : public IDocumentService {
public:
    using ErrorListCallback = std::function<void(const std::vector<std::pair<uint32_t, uint32_t>>&)>;

    SpellCheckAnalyzer(std::shared_ptr<SpellCheckerService> spell_checker, std::string default_lang, ErrorListCallback on_complete);

    void analyze(std::shared_ptr<DocumentSnapshot> snapshot) override;
    std::string getName() const override { return "SpellCheckAnalyzer"; }

private:
    std::shared_ptr<SpellCheckerService> spell_checker_;
    std::string default_lang_;
    ErrorListCallback on_complete_;
};

} // namespace pluma
