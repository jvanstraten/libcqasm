#include "cqasm-analyzer.hpp"
#include "cqasm-parse-helper.hpp"

namespace cqasm {

/**
 * Analyzes the given cQASM file.
 */
AnalysisResult Analyzer::analyze(const std::string &filename) {
    AnalysisResult result;
    result.filename = filename;

    auto parser = parser::ParseHelper(filename);
    result.errors = std::move(parser.errors);
    result.ast_root = std::move(parser.root);

    return result;
}

} // namespace cqasm
