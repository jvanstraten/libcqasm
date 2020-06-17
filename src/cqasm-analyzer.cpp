#include "cqasm-analyzer.hpp"
#include "cqasm-parser.hpp"
#include "cqasm-lexer.hpp"

namespace cqasm {

/**
 * Construct the analyzer internals for the given filename, and analyze
 * the file.
 */
ParseHelper::ParseHelper(const std::string &filename) {
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

/**
 * Destroys the analyzer.
 */
ParseHelper::~ParseHelper() {
    if (fptr) {
        fclose(fptr);
    }
    if (scanner) {
        yylex_destroy((yyscan_t)scanner);
    }
}

/**
 * Pushes an error.
 */
void ParseHelper::push_error(const std::string &error) {
    result.errors.push_back(error);
}

/**
 * Analyzes the given cQASM file.
 */
AnalysisResult Analyzer::analyze(const std::string &filename) {
    return ParseHelper(filename).result;
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

} // namespace cqasm

/**
 * Stream << overload for source location objects.
 */
std::ostream& operator<<(std::ostream& os, const cqasm::SourceLocation& object) {

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
