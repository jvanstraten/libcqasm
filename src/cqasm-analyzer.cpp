#include "cqasm-analyzer.hpp"
#include "cqasm-parser.hpp"
#include "cqasm-lexer.hpp"

namespace cqasm {

AnalyzerInternals::AnalyzerInternals(const std::string &filename) {
    int retcode;

    // Store the filename.
    result.filename = filename;

    // Create the scanner.
    retcode = yylex_init((yyscan_t*)&scanner);
    if (retcode) {
        std::ostringstream sb;
        sb << "Failed to construct scanner: " << strerror(retcode);
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
    retcode = yyparse((yyscan_t)scanner, *this);
    if (retcode == 2) {
        std::ostringstream sb;
        sb << "Out of memory while parsing " << filename;
        push_error(sb.str());
        return;
    } else if (retcode) {
        std::ostringstream sb;
        sb << "Failed to parse " << filename;
        push_error(sb.str());
        return;
    }

}

AnalyzerInternals::~AnalyzerInternals() {
    if (fptr) {
        fclose(fptr);
    }
    if (scanner) {
        yylex_destroy((yyscan_t)scanner);
    }
}

void AnalyzerInternals::push_error(const std::string &error) {
    result.errors.push_back(error);
}

AnalysisResult Analyzer::analyze(const std::string &filename) {
    return AnalyzerInternals(filename).result;
}

} // namespace cqasm
