#include "cqasm-ast.hpp"
#include <stdexcept>

namespace cqasm {
namespace ast {

/**
 * Dumps this node.
 */
void Node::dump(std::ostream &out) {
    auto dumper = Dumper(out);
    visit(dumper);
}

/**
 * Pushes a string fragment into the string.
 */
void StringBuilder::push_string(const std::string &str) {
    stream << str;
}

/**
 * Pushes an escape sequence into the string.
 */
void StringBuilder::push_escape(const std::string &escape) {
    if (escape == "\\t") {
        stream << '\t';
    } else if (escape == "\\n") {
        stream << '\n';
    } else if (escape == "\\r") {
        stream << '\r';
    } else if (escape == "\\'") {
        stream << '\'';
    } else if (escape == "\\\"") {
        stream << '\"';
    } else if (escape == "\\\\") {
        stream << '\\';
    } else {
        stream << escape;
    }
}

/**
 * Visit this object.
 */
void StringBuilder::visit(Visitor &visitor) {
    throw std::runtime_error("Cannot visit StringBuilders");
}

/**
 * Constructs a source location object.
 */
SourceLocation::SourceLocation(
    const std::string &filename,
    uint32_t first_line,
    uint32_t first_column,
    uint32_t last_line,
    uint32_t last_column
) :
    filename(filename),
    first_line(first_line),
    first_column(first_column),
    last_line(last_line),
    last_column(last_column)
{
    if (last_line < first_line) {
        last_line = first_line;
    }
    if (last_line == first_line && last_column < first_column) {
        last_column = first_column;
    }
}

/**
 * Expands the location range to contain the given location in the source
 * file.
 */
void SourceLocation::expand_to_include(uint32_t line, uint32_t column) {
    if (line < first_line) {
        first_line = line;
    }
    if (line == first_line && column < first_column) {
        first_column = column;
    }
    if (line > last_line) {
        last_line = line;
    }
    if (line == last_line && column > last_column) {
        last_column = column;
    }
}

} // namespace ast
} // namespace cqasm

/**
 * Stream << overload for source location objects.
 */
std::ostream& operator<<(std::ostream& os, const cqasm::ast::SourceLocation& object) {

    // Print filename.
    os << object.filename;

    // Special case for when only the source filename is known.
    if (!object.first_line) {
        return os;
    }

    // Print line number.
    os << ":" << object.first_line;

    // Special case for when only line numbers are known.
    if (!object.first_column) {

        // Print last line too, if greater.
        if (object.last_line > object.first_line) {
            os << ".." << object.last_line;
        }

        return os;
    }

    // Print column.
    os << ":" << object.first_column;

    if (object.last_line == object.first_line) {

        // Range is on a single line. Only repeat the column number.
        if (object.last_column > object.first_column) {
            os << ".." << object.last_column;
        }

    } else if (object.last_line > object.first_line) {

        // Range is on multiple lines. Repeat both line and column number.
        os << ".." << object.last_line;
        if (object.last_column > object.first_column) {
            os << ":" << object.last_column;
        }

    }

    return os;
}

/**
 * Stream << overload for AST nodes (writes debug dump).
 */
std::ostream& operator<<(std::ostream& os, const cqasm::ast::Node& object) {
    const_cast<cqasm::ast::Node&>(object).dump(os);
    return os;
}
