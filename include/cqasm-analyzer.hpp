#ifndef _CQASM_ANALYZER_HPP_INCLUDED_
#define _CQASM_ANALYZER_HPP_INCLUDED_

#include "cqasm-ast.hpp"
#include <cstdio>

namespace cqasm {

/**
 * Analysis result class.
 */
class AnalysisResult {
public:

    /**
     * Name of the file that was analyzed.
     */
    std::string filename;

    /**
     * List of accumulated errors. Analysis was successeful if and only if
     * `errors.empty()`.
     */
    std::vector<std::string> errors;

    /**
     * Root node of the abstract syntax tree, if parsing was sufficiently
     * successful.
     */
    ast::One<ast::Root> ast_root;

};

/**
 * Internal helper class for analyzing cQASM files. Don't use this directly,
 * use Analyzer instead.
 *
 * @see Analyzer
 */
class ParseHelper {
public:

    /**
     * File pointer being scanned.
     */
    FILE *fptr = nullptr;

    /**
     * Flex reentrant scanner data.
     */
    void *scanner = nullptr;

    /**
     * Analysis result.
     */
    AnalysisResult result;

private:
    friend class Analyzer;

    /**
     * Construct the parse helper for the given filename, and analyze the file
     * with flex/bison.
     */
    ParseHelper(const std::string &filename);

public:

    /**
     * Destroys the parse helper.
     */
    virtual ~ParseHelper();

    /**
     * Pushes an error.
     */
    void push_error(const std::string &error);

};

/**
 * Main class used for analyzing cQASM files.
 */
class Analyzer {
public:

    /**
     * Analyzes the given cQASM file.
     */
    AnalysisResult analyze(const std::string &filename);

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

} // namespace cqasm

/**
 * Stream << overload for source location objects.
 */
std::ostream& operator<<(std::ostream& os, const cqasm::SourceLocation& object);

#endif
