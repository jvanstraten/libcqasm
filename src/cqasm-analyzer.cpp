#include "cqasm-analyzer.hpp"
#include "cqasm-parser.hpp"
#include "cqasm-lexer.hpp"

namespace cqasm {

Analyzer::Analyzer(const std::string &filename) {
  int result;

  // Store the filename.
  this->filename = filename;

  // Create the scanner.
  result = yylex_init((yyscan_t*)&scanner);
  if (result) {
    std::ostringstream sb;
    sb << "Failed to construct scanner: " << strerror(result);
    push_error(sb.str());
    return;
  }

  // Try to open the file and read it to an internal string.
  fptr = fopen(filename.c_str(), "r");
  if (!fptr) {
    std::ostringstream sb;
    sb << "Failed to open input file " << filename << ": " << strerror(errno);
    push_error(sb.str());
    return;
  }
  yyset_in(fptr, (yyscan_t)scanner);

  // Do the actual parsing.
  result = yyparse((yyscan_t)scanner, *this);
  if (result == 2) {
    std::ostringstream sb;
    sb << "Out of memory while parsing " << filename;
    push_error(sb.str());
    return;
  } else if (result) {
    std::ostringstream sb;
    sb << "Failed to parse " << filename;
    push_error(sb.str());
    return;
  }

}

Analyzer::~Analyzer() {
  if (fptr) {
    fclose(fptr);
  }
  if (scanner) {
    yylex_destroy((yyscan_t)scanner);
  }
}

void Analyzer::push_error(const std::string &error) {
  errors.push_back(error);
}

} // namespace cqasm
