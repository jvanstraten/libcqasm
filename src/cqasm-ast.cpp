#include "cqasm-ast.hpp"
#include <stdexcept>

namespace cqasm {
namespace ast {

/**
 * Dumps this node.
 */
void Base::dump(std::ostream &out) {
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

} // namespace ast
} // namespace cqasm

/**
 * Stream << overload for AST nodes (writes debug dump).
 */
std::ostream& operator<<(std::ostream& os, const cqasm::ast::Base& object) {
    const_cast<cqasm::ast::Base&>(object).dump(os);
    return os;
}
