#pragma once

#include "cqasm-ast.hpp"
#include "cqasm-semantic.hpp"
#include "cqasm-resolver.hpp"
#include <cstdio>

namespace cqasm {
namespace analyzer {

/**
 * Analysis result class.
 */
class AnalysisResult {
public:

    /**
     * Root node of the semantic tree, if analysis was successful.
     */
    ast::One<semantic::Program> root;

    /**
     * List of accumulated errors. Analysis was successful if and only if
     * `errors.empty()`.
     */
    std::vector<std::string> errors;

};

/**
 * Main class used for analyzing cQASM files.
 */
class Analyzer {
private:
    friend class AnalyzerHelper;

    /**
     * The set of "mappings" that the parser starts out with (map statements in
     * the cQASM code mutate a local copy of this).
     */
    resolver::MappingTable mappings;

    /**
     * The supported set of classical functions and operators. Functions have a
     * name (either a case-insensitively matched function name using the usual
     * function notation, or one of the supported operators), a signature
     * for the types of arguments it expects, and a C++ function that takes
     * value nodes of those expected types and returns the resulting value.
     * Note that, once runtime expressions are implemented, the resulting value
     * can be some expression of the incoming values.
     */
    resolver::FunctionTable functions;

    /**
     * The supported set of quantum/classical/mixed instructions, appearing in
     * the cQASM file as assembly-like commands. Instructions have a
     * case-insensitively matched name, a signature for the types of parameters
     * it expects, and some flags indicating how (much) error checking is to
     * be done. You can also add your own metadata through the Annotatable
     * interface.
     */
    resolver::InstructionTable instruction_set;

    /**
     * When set, instruction resolution is disabled. That is, instruction_set
     * is unused, no type promotion is (or can be) performed for instruction
     * parameters, and the instruction field of the semantic::Instruction nodes
     * is left uninitialized.
     */
    bool resolve_instructions;

    /**
     * The supported set of error models. Zero or one of these can be specified
     * in the cQASM file using the special "error_model" instruction. Error
     * models have a name and a signature for the types of parameters it
     * expects. You can also add your own metadata through the Annotatable
     * interface.
     */
    resolver::ErrorModelTable error_models;

    /**
     * When set, error model resolution is disabled. That is, error_models
     * is unused, no type promotion is (or can be) performed for instruction
     * parameters, and the model field of the semantic::ErrorModel node is left
     * uninitialized.
     */
    bool resolve_error_model;

public:

    /**
     * Analyzes the given program AST node.
     */
    AnalysisResult analyze(const ast::Program &program) const;

};

} // namespace analyzer
} // namespace cqasm
