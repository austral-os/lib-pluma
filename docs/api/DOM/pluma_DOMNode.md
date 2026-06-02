# Class `pluma::DOMNode`

**Abstract base class for all nodes in the logical document tree.**

## Public Methods
- `DOMNode(NodeId id)` - *Constructs a node with a unique ID.*
- `~DOMNode()=default`
- `NodeType getType() const =0` - *Gets the structural type of the node.*
- `NodeId getId() const` - *Retrieves the unique ID of the node.*
- `void addChild(std::unique_ptr< DOMNode > child)` - *Appends a child to this node.*
- `const std::vector< std::unique_ptr< DOMNode > > & getChildren() const` - *Gets all children of this node.*
- `PropertyBag & getStyle()` - *Gets the declared styles for this node.*
- `const PropertyBag & getStyle() const` - *Gets the declared styles for this node (read-only).*

