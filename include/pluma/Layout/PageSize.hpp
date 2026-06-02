/**
 * @file PageSize.hpp
 * @brief Standard definitions and structures for document page dimensions.
 */
#pragma once
#include <pluma/CoreTypes.hpp>

namespace pluma {

/**
 * @struct PageSize
 * @brief Represents the physical dimensions of a page.
 */
struct PageSize {
    Twips width;
    Twips height;

    PageSize() : width(0), height(0) {}
    PageSize(Twips w, Twips h) : width(w), height(h) {}
};

/**
 * @struct PageMargins
 * @brief Represents the physical margins of a document page.
 */
struct PageMargins {
    Twips top;
    Twips bottom;
    Twips left;
    Twips right;

    PageMargins() 
        : top(1440), bottom(1440), left(1440), right(1440) {} // Default 1 inch
        
    PageMargins(Twips t, Twips b, Twips l, Twips r) 
        : top(t), bottom(b), left(l), right(r) {}
};

/**
 * @namespace PageSizes
 * @brief Commonly used industry standard page dimensions.
 */
namespace PageSizes {
    // ISO A-Series
    const PageSize A3(Twips(16838), Twips(23811));
    const PageSize A4(Twips(11906), Twips(16838));
    const PageSize A5(Twips(8391), Twips(11906));

    // North American
    const PageSize Letter(Twips(12240), Twips(15840));  // 8.5" x 11"
    const PageSize Legal(Twips(12240), Twips(20160));   // 8.5" x 14"
    const PageSize Tabloid(Twips(15840), Twips(24480)); // 11" x 17"
}

} // namespace pluma
