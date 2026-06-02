/**
 * @file DOMNode.hpp
 * @brief Defines the logical Document Object Model tree structure.
 */
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <pluma/NodeId.hpp>
#include <pluma/Style/PropertyBag.hpp>

namespace pluma {

/**
 * @enum NodeType
 * @brief Categorizes logical nodes in the document tree.
 */
enum class NodeType {
    Document,  ///< The root node of the entire document.
    Paragraph, ///< A block-level container separated by line breaks.
    Run,       ///< An inline sequence of text sharing the same styling.
    Table,     ///< A table container.
    TableRow,  ///< A row within a table.
    TableCell, ///< A cell within a table row.
    Image      ///< An image element.
};

/**
 * @class DOMNode
 * @brief Abstract base class for all nodes in the logical document tree.
 */
class DOMNode {
public:
    /**
     * @brief Constructs a node with a unique ID.
     * @param id The unique identifier.
     */
    explicit DOMNode(NodeId id) : id_(id) {}
    virtual ~DOMNode() = default;

    /**
     * @brief Gets the structural type of the node.
     * @return The NodeType.
     */
    virtual NodeType getType() const = 0;

    /**
     * @brief Retrieves the unique ID of the node.
     * @return The NodeId.
     */
    NodeId getId() const { return id_; }

    /**
     * @brief Appends a child to this node.
     * @param child Unique pointer to the child node.
     */
    void addChild(std::unique_ptr<DOMNode> child) {
        children_.push_back(std::move(child));
    }

    /**
     * @brief Gets all children of this node.
     * @return Read-only reference to the children list.
     */
    const std::vector<std::unique_ptr<DOMNode>>& getChildren() const {
        return children_;
    }

    /**
     * @brief Gets the declared styles for this node.
     * @return Reference to the PropertyBag.
     */
    PropertyBag& getStyle() { return style_; }

    /**
     * @brief Gets the declared styles for this node (read-only).
     * @return Read-only reference to the PropertyBag.
     */
    const PropertyBag& getStyle() const { return style_; }

private:
    NodeId id_;
    std::vector<std::unique_ptr<DOMNode>> children_;
    PropertyBag style_;
};

/**
 * @class DocumentNode
 * @brief The root of the DOM tree.
 */
class DocumentNode : public DOMNode {
public:
    explicit DocumentNode(NodeId id) : DOMNode(id) {}
    NodeType getType() const override { return NodeType::Document; }
};

/**
 * @class ParagraphNode
 * @brief Represents a logical paragraph.
 */
class ParagraphNode : public DOMNode {
public:
    explicit ParagraphNode(NodeId id) : DOMNode(id) {}
    NodeType getType() const override { return NodeType::Paragraph; }
};

/**
 * @class RunNode
 * @brief Represents a contiguous span of styled text within a paragraph.
 */
class RunNode : public DOMNode {
public:
    /**
     * @brief Constructs a RunNode pointing to a specific text span.
     * @param id The unique identifier.
     * @param logical_offset The start index in the PieceTable.
     * @param length The number of characters/bytes.
     */
    RunNode(NodeId id, uint32_t logical_offset, uint32_t length)
        : DOMNode(id), offset_(logical_offset), length_(length) {}

    NodeType getType() const override { return NodeType::Run; }

    uint32_t getOffset() const { return offset_; }
    uint32_t getLength() const { return length_; }

private:
    uint32_t offset_;
    uint32_t length_;
};

/**
 * @class TableNode
 * @brief Represents a table container.
 */
class TableNode : public DOMNode {
public:
    explicit TableNode(NodeId id) : DOMNode(id) {}
    NodeType getType() const override { return NodeType::Table; }
};

/**
 * @class TableRowNode
 * @brief Represents a row in a table.
 */
class TableRowNode : public DOMNode {
public:
    explicit TableRowNode(NodeId id) : DOMNode(id) {}
    NodeType getType() const override { return NodeType::TableRow; }
};

/**
 * @class TableCellNode
 * @brief Represents a cell in a table.
 */
class TableCellNode : public DOMNode {
public:
    explicit TableCellNode(NodeId id) : DOMNode(id) {}
    NodeType getType() const override { return NodeType::TableCell; }
};

} // namespace pluma
