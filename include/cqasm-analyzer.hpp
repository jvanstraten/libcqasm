#pragma once

#include "cqasm-ast.hpp"
#include "cqasm-semantic.hpp"
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

    /**
     * Root node of the semantic tree, if analysis was successful.
     */
    ast::One<semantic::Root> root;

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
