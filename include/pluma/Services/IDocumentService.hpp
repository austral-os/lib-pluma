/**
 * @file IDocumentService.hpp
 * @brief Interface for background document analysis services.
 */
#pragma once
#include <memory>
#include <string>
#include <pluma/PieceTable.hpp>

namespace pluma {

/**
 * @class IDocumentService
 * @brief A service that performs asynchronous or decoupled analysis on a DocumentSnapshot.
 */
class IDocumentService {
public:
    virtual ~IDocumentService() = default;
    
    /**
     * @brief Performs analysis on a stable snapshot.
     * @param snapshot The immutable state of the document.
     */
    virtual void analyze(std::shared_ptr<DocumentSnapshot> snapshot) = 0;
    
    /**
     * @brief Returns the internal name of the service.
     */
    virtual std::string getName() const = 0;
};

} // namespace pluma
