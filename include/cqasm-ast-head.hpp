#ifndef _CQASM_AST_HEAD_HPP_INCLUDED_
#define _CQASM_AST_HEAD_HPP_INCLUDED_

#include <cstdint>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include "cqasm-tree.hpp"

namespace cqasm {
namespace ast {

// Base classes used to construct the AST.
USING_CQASM_TREE;

// Forward declaration for the NodeType enum.
enum class NodeType;

// Forward declaration for the Visitor class defined in cqasm-ast-gen.hpp.
class Visitor;

/**
 * String primitive used within the AST.
 */
using Str = std::string;

/**
 * Integer primitive used within the AST.
 */
using Int = std::int64_t;

/**
 * Real number primitive used within the AST.
 */
using Real = double;

/**
 * Main class for AST nodes.
 */
class Node : public Base {
public:

    /**
     * Returns the `NodeType` of this node.
     */
    virtual NodeType type() const = 0;

    /**
     * Equality operator. Ignores annotations!
     */
    virtual bool operator==(const Node& rhs) const = 0;

    /**
     * Inequality operator. Ignores annotations!
     */
    inline bool operator!=(const Node& rhs) const {
        return !(*this == rhs);
    }

    /**
     * Visit this object.
     */
    virtual void visit(Visitor &visitor) = 0;

    /**
     * Writes a debug dump of this object to the given stream.
     */
    void dump(std::ostream &out=std::cout);

};

/**
 * Special/temporary string builder node, used to build strings from fragments
 * and escape sequences while parsing. This is abstracted out of the AST; it
 * should never appear after parsing completes.
 */
class StringBuilder : public Base {
public:
    std::ostringstream stream;

    /**
     * Pushes a string fragment into the string.
     */
    void push_string(const std::string &str);

    /**
     * Pushes an escape sequence into the string.
     */
    void push_escape(const std::string &escape);

};

} // namespace ast
} // namespace cqasm

/**
 * Stream << overload for AST nodes (writes debug dump).
 */
std::ostream& operator<<(std::ostream& os, const cqasm::ast::Node& object);

#endif
