#include "cqasm-ast.hpp"

namespace cqasm {
namespace ast {

void MatrixLiteral::set_raw_square_pairs(ExpressionList *square_pairs) {
}

void StringBuilder::push_string(const std::string &str) {
  stream << str;
}

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

} // namespace ast
} // namespace cqasm
