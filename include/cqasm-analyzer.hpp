#ifndef _CQASM_ANALYZER_HPP_INCLUDED_
#define _CQASM_ANALYZER_HPP_INCLUDED_

#include "cqasm-ast.hpp"
#include <cstdio>

namespace cqasm {

class Analyzer {
private:
  std::string filename = "";
  void *scanner = nullptr;
  FILE *fptr = nullptr;
public:
  std::vector<std::string> errors;
  Analyzer(const std::string &filename);
  ~Analyzer();
  void push_error(const std::string &error);

};

} // namespace cqasm

#endif
