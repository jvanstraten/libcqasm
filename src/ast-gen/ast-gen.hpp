#ifndef _AST_GEN_HPP_INCLUDED_
#define _AST_GEN_HPP_INCLUDED_

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>
#include <cctype>

/**
 * Repetition modes for AST nodes.
 */
enum ChildType {

    /**
     * Zero or one nodes.
     */
    Maybe,

    /**
     * Exactly one nodes.
     */
    One,

    /**
     * Zero or more nodes.
     */
    Any,

    /**
     * One or more nodes.
     */
    Many,

    /**
     * String primitive.
     */
    Str,

    /**
     * Integer primitive.
     */
    Int,

    /**
     * Real number primitive.
     */
    Real,

    /**
     * Vector of integers representing a version.
     */
    Version,

};

struct NodeType;

/**
 * Represents a child node.
 */
struct ChildNode {

    /**
     * The type of child node.
     */
    ChildType type;

    /**
     * The child node type, if any (depends on type).
     */
    std::shared_ptr<NodeType> node_type;

    /**
     * Class member name.
     */
    std::string name;

    /**
     * Class member documentation.
     */
    std::string doc;

};

/**
 * Represents a type of AST node.
 */
struct NodeType {

    /**
     * Name in snake_case.
     */
    std::string snake_case_name;

    /**
     * Name in TitleCase.
     */
    std::string title_case_name;

    /**
     * Class documentation.
     */
    std::string doc;

    /**
     * The node type this is derived from, if any.
     */
    std::shared_ptr<NodeType> parent;

    /**
     * Node types derived from this one.
     */
    std::vector<std::weak_ptr<NodeType>> derived;

    /**
     * Child nodes.
     */
    std::vector<ChildNode> children;

    /**
     * Whether this node represents a recovered parse error.
     */
    bool is_error_marker;

    /**
     * Gathers all child nodes, including those in parent classes.
     */
    std::vector<ChildNode> all_children() {
        std::vector<ChildNode> children = this->children;
        if (parent) {
            auto from_parent = parent->all_children();
            children.insert(children.end(), from_parent.begin(), from_parent.end());
        }
        return children;
    }

};

/**
 * List of nodes.
 */
using Nodes = std::vector<std::shared_ptr<NodeType>>;

/**
 * Convenience class for constructing a node.
 */
class NodeBuilder {
private:

    /**
     * The node being constructed.
     */
    std::shared_ptr<NodeType> node;

public:

    /**
     * Construct a node with the given snake_case name and class documentation.
     */
    NodeBuilder(const std::string &name, const std::string &doc="") {
        node = std::make_shared<NodeType>();
        node->snake_case_name = name;
        node->doc = doc;
        node->is_error_marker = false;

        // Generate title case name.
        auto snake_ss = std::stringstream(name);
        auto title_ss = std::ostringstream();
        std::string token;
        while (std::getline(snake_ss, token, '_')) {
            title_ss << (char)std::toupper(token[0]) << token.substr(1);
        }
        node->title_case_name = title_ss.str();
    }

    /**
     * Marks this node as deriving from the given node type.
     */
    NodeBuilder &&derive_from(std::shared_ptr<NodeType> parent) {
        node->parent = parent;
        parent->derived.push_back(node);
        return std::move(*this);
    }

    /**
     * Adds a child node. `type` should be `One`, `Maybe`, or `Many`.
     */
    NodeBuilder &&with(
        ChildType type,
        std::shared_ptr<NodeType> &node_type,
        const std::string &name,
        const std::string &doc = ""
    ) {
        auto child = ChildNode();
        child.type = type;
        child.node_type = node_type;
        child.name = name;
        child.doc = doc;
        node->children.push_back(std::move(child));
        return std::move(*this);
    }

    /**
     * Adds a child primitive. `type` should be `Str`, `Int`, `Real`, or
     * `Version`.
     */
    NodeBuilder &&with(
        ChildType type,
        const std::string &name,
        const std::string &doc = ""
    ) {
        auto child = ChildNode();
        child.type = type;
        child.name = name;
        child.doc = doc;
        node->children.push_back(std::move(child));
        return std::move(*this);
    }

    /**
     * Indicate that this node marks a recovered parse error.
     */
    NodeBuilder &&marks_error() {
        node->is_error_marker = true;
        return std::move(*this);
    }

    /**
     * Returns the constructed node.
     */
    std::shared_ptr<NodeType> build(Nodes &nodes) {
        nodes.push_back(node);
        return std::move(node);
    }

};

/**
 * Constructs the cQASM AST nodes.
 */
Nodes build_nodes();

#endif
