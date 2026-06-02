/**
 * @file Dumper.hpp
 * @brief Provides textual dump methods for internal data structures to assist in debugging.
 */
#pragma once
#include <string>
#include <memory>
#include <vector>
#include <pluma/DOM/DOMNode.hpp>
#include <pluma/Layout/LayoutBox.hpp>

namespace pluma {
namespace diagnostics {

/**
 * @class Dumper
 * @brief Static utility class for converting complex in-memory structures into string trees.
 */
class Dumper {
public:
    /**
     * @brief Recursively generates a string representation of the DOM tree.
     * @param root The root node to dump.
     * @param depth Current recursion depth (used for indentation).
     * @return Formatted string showing DOM hierarchy.
     */
    static std::string dumpDOM(const DOMNode& root, int depth = 0);

    /**
     * @brief Recursively generates a string representation of the Layout box tree.
     * @param pages The list of top-level PageBox items.
     * @return Formatted string showing Layout hierarchy and bounds.
     */
    static std::string dumpLayout(const std::vector<std::unique_ptr<PageBox>>& pages);

private:
    static std::string dumpLayoutBox(const LayoutBox& box, int depth);
};

} // namespace diagnostics
} // namespace pluma
