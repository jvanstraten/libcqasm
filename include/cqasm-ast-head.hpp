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
class StringBuilder : public Node {
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

    /**
     * Visit this object.
     */
    void visit(Visitor &visitor) override;

};

/**
 * Source location annotation object, containing source file line numbers etc.
 */
class SourceLocation {
public:

    /**
     * The name of the source file.
     */
    std::string filename;

    /**
     * The first line of the range, or 0 if unknown.
     */
    uint32_t first_line;

    /**
     * The first column of the range, or 0 if unknown.
     */
    uint32_t first_column;

    /**
     * The last line of the range, or 0 if unknown.
     */
    uint32_t last_line;

    /**
     * The last column of the range, or 0 if unknown.
     */
    uint32_t last_column;

    /**
     * Constructs a source location object.
     */
    SourceLocation(
        const std::string &filename,
        uint32_t first_line = 0,
        uint32_t first_column = 0,
        uint32_t last_line = 0,
        uint32_t last_column = 0
    );

    /**
     * Expands the location range to contain the given location in the source
     * file.
     */
    void expand_to_include(uint32_t line, uint32_t column = 1);

};

} // namespace ast
} // namespace cqasm

/**
 * Stream << overload for source location objects.
 */
std::ostream& operator<<(std::ostream& os, const cqasm::ast::SourceLocation& object);

/**
 * Stream << overload for AST nodes (writes debug dump).
 */
std::ostream& operator<<(std::ostream& os, const cqasm::ast::Node& object);

#endif
