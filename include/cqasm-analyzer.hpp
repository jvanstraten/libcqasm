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
class AnalyzerInternals {
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
     * Construct the analyzer internals for the given filename, and analyze
     * the file.
     */
    AnalyzerInternals(const std::string &filename);

public:

    /**
     * Destroys the analyzer.
     */
    virtual ~AnalyzerInternals();

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

} // namespace cqasm

#endif
