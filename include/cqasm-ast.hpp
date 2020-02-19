#ifndef _CQASM_AST_HPP_INCLUDED_
#define _CQASM_AST_HPP_INCLUDED_

#include <stdexcept>
#include "cqasm-ast-util.hpp"
#include "cqasm-ast-gen.hpp"

namespace cqasm {
namespace ast {

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

} // namespace ast
} // namespace cqasm

#endif
