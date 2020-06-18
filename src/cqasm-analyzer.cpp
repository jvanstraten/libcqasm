#include <cqasm-parse-helper.hpp>
#include "cqasm-analyzer.hpp"

namespace cqasm {
namespace analyzer {

/**
 * Exception for failed name resolutions.
 */
class InvalidMatrixLiteral : std::exception {
public:
    const char *what() const noexcept override {
        return "invalid matrix literal";
    }
};

/**
 * Exception for failed indexation.
 */
class IndexOperatorError : std::exception {
public:
    const char *what() const noexcept override {
        return "indexation failed";
    }
};

/**
 * Exception for failed parsing of the number of qubits.
 */
class NumQubitsError : std::exception {
public:
    const char *what() const noexcept override {
        return "invalid number of qubits";
    }
};

/**
 * Parses the given expression into a value using the given mapping scope.
 */
values::Value Analyzer::parse_expression(
    const ast::Expression &expression,
    const resolver::MappingTable &scope
) const {
    values::Value retval;
    if (auto int_lit = expression.as_integer_literal()) {
        retval.set(tree::make<values::ConstInt>(int_lit->value));
    } else if (auto float_lit = expression.as_float_literal()) {
        retval.set(tree::make<values::ConstReal>(float_lit->value));
    } else if (auto string_lit = expression.as_string_literal()) {
        retval.set(tree::make<values::ConstString>(string_lit->value));
    } else if (auto json_lit = expression.as_json_literal()) {
        retval.set(tree::make<values::ConstJson>(json_lit->value));
    } else if (auto matrix_lit = expression.as_matrix_literal()) {

        // Figure out the size of the matrix and parse the subexpressions.
        // Note that the number of rows is always at least 1 (Many vs Any) so
        // the ncols line is well-behaved.
        size_t nrows = matrix_lit->rows.size();
        size_t ncols = matrix_lit->rows[0].size();
        std::vector<values::Value> vals;
        for (size_t row = 0; row < nrows; row++) {
            for (size_t col = 0; col < ncols; col++) {
                vals.push_back(
                    parse_expression(*matrix_lit->rows[row]->items[col],
                                     scope));
            }
        }

        // First try to make a matrix of reals.
        auto rmatrix = primitives::RMatrix(nrows, ncols);
        bool success = true;
        for (size_t row = 0; row < nrows; row++) {
            for (size_t col = 0; col < ncols; col++) {
                if (auto val = values::promote(vals[row * ncols + col],
                                               tree::make<types::Real>())) {
                    // Only constants are supported.
                    if (auto val_real = val->as_const_real()) {
                        rmatrix.at(row + 1, col + 1) = val_real->value;
                        continue;
                    }
                }
                success = false;
                break;
            }
            if (!success) {
                break;
            }
        }
        if (success) {
            retval.set(tree::make<values::ConstRealMatrix>(rmatrix));
        } else {

            // Failed to make a matrix of reals, try complex instead.
            auto cmatrix = primitives::CMatrix(nrows, ncols);
            success = true;
            for (size_t row = 0; row < nrows; row++) {
                for (size_t col = 0; col < ncols; col++) {
                    if (auto val = values::promote(vals[row * ncols + col],
                                                   tree::make<types::Complex>())) {
                        // Only constants are supported.
                        if (auto val_complex = val->as_const_complex()) {
                            cmatrix.at(row + 1, col + 1) = val_complex->value;
                            continue;
                        }
                    }
                    success = false;
                    break;
                }
                if (!success) {
                    break;
                }
            }
            if (success) {
                retval.set(tree::make<values::ConstComplexMatrix>(cmatrix));
            } else {

                // Only real and complex are supported right now. If more is to be
                // added in the future, this should probably be written a little
                // neater.
                throw InvalidMatrixLiteral();

            }
        }

    } else if (auto ident = expression.as_identifier()) {

        // All identifiers are to be looked up in the table of mappings. q and
        // b resolve this way because they are added as default mappings to
        // the full qubit and measurement registers respectively.
        retval.set(scope.resolve(ident->name));

    } else if (auto index = expression.as_index()) {

        // While matrices could conceivably be indexed, they aren't right now.
        // We just support indexation of QubitRefs and BitRefs.
        auto expr = parse_expression(*index->expr, scope);
        tree::Many<values::ConstInt> current_indices;
        if (auto qubit_refs = expr->as_qubit_refs()) {
            current_indices = qubit_refs->index;
        } else if (auto bit_refs = expr->as_bit_refs()) {
            current_indices = bit_refs->index;
        } else {
            // Only QubitRefs and BitRefs can be indexed.
            throw IndexOperatorError();
        }

        // Slice the current_indices list accordingly.
        tree::Many<values::ConstInt> new_indices;
        for (auto entry : index->indices->items) {
            if (auto item = entry->as_index_item()) {

                // Parse index subexpression.
                auto ie = parse_expression(*item->index, scope);
                auto iv = values::promote(ie, tree::make<types::Int>());
                if (!iv) {
                    // Index must be an integer.
                    throw IndexOperatorError();
                }
                auto ic = iv->as_const_int();
                if (!ic) {
                    // Index must be constant.
                    throw IndexOperatorError();
                }
                auto i = ic->value;
                if (i < 0 || (size_t) i >= current_indices.size()) {
                    // Index is out of range.
                    throw IndexOperatorError();
                }

                // Add the index.
                auto idx = tree::make<values::ConstInt>(current_indices[i]);
                idx->copy_annotation<parser::SourceLocation>(*item);
                new_indices.add(idx);

            } else if (auto range = entry->as_index_range()) {

                // Parse first index subexpression.
                auto fe = parse_expression(*range->first, scope);
                auto fv = values::promote(fe, tree::make<types::Int>());
                if (!fv) {
                    // Index must be an integer.
                    throw IndexOperatorError();
                }
                auto fc = fv->as_const_int();
                if (!fc) {
                    // Index must be constant.
                    throw IndexOperatorError();
                }
                auto f = fc->value;

                // Parse last index subexpression.
                auto le = parse_expression(*range->first, scope);
                auto lv = values::promote(le, tree::make<types::Int>());
                if (!lv) {
                    // Index must be an integer.
                    throw IndexOperatorError();
                }
                auto lc = lv->as_const_int();
                if (!lc) {
                    // Index must be constant.
                    throw IndexOperatorError();
                }
                auto l = lc->value;

                // Check range.
                if (f < 0 || l < f || (size_t) l >= current_indices.size()) {
                    // Index is out of range.
                    throw IndexOperatorError();
                }

                // Add the indices.
                for (size_t i = (size_t) f; i <= (size_t) l; i++) {
                    auto idx = tree::make<values::ConstInt>(current_indices[i]);
                    idx->copy_annotation<parser::SourceLocation>(*range);
                    new_indices.add(idx);
                }

            } else {
                throw std::runtime_error("unknown IndexEntry AST node");
            }
        }

        // Construct the new reference node.
        if (expr->as_qubit_refs()) {
            retval.set(tree::make<values::QubitRefs>(new_indices));
        } else if (expr->as_bit_refs()) {
            retval.set(tree::make<values::BitRefs>(new_indices));
        }

    } else if (auto func = expression.as_function_call()) {
        // TODO
    } else if (auto negate = expression.as_negate()) {
        // TODO
    } else if (auto power = expression.as_power()) {
        // TODO
    } else if (auto mult = expression.as_multiply()) {
        // TODO
    } else if (auto div = expression.as_divide()) {
        // TODO
    } else if (auto add = expression.as_add()) {
        // TODO
    } else if (auto sub = expression.as_subtract()) {
        // TODO
    } else {
        throw std::runtime_error("unexpected expression node");
    }
    if (!retval) {
        throw std::runtime_error(
            "parse_expression returned nonsense, this should never happen");
    }
    retval->copy_annotation<parser::SourceLocation>(expression);
    return retval;
}

/**
 * Analyzes the given AST.
 */
AnalysisResult Analyzer::analyze(const ast::Program &ast) const {
    AnalysisResult result;

    // Construct the program node and copy the file version into it.
    result.root.set(tree::make<semantic::Program>());
    result.root->copy_annotation<parser::SourceLocation>(ast);
    result.root->version = tree::make<semantic::Version>(ast.version->items);
    result.root->version->copy_annotation<parser::SourceLocation>(*ast.version);

    // Construct a scope from the initial set of mappings.
    auto scope = resolver::MappingTable(mappings);

    // Figure out the number of qubits.
    auto nqe = parse_expression(*ast.num_qubits, scope);
    auto nqv = values::promote(nqe, tree::make<types::Int>());
    if (!nqv) {
        // Number of qubits must be an integer.
        throw NumQubitsError();
    }
    auto nqc = nqv->as_const_int();
    if (!nqc) {
        // Number of qubits must be constant.
        throw NumQubitsError();
    }
    result.root->num_qubits = nqc->value;
    if (result.root->num_qubits < 1) {
        // Number of qubits must be positive.
        throw NumQubitsError();
    }

    // Construct the special q and b mappings, that map to the whole qubit and
    // measurement register respectively.
    tree::Many<values::ConstInt> all_qubits;
    for (primitives::Int i = 0; i < result.root->num_qubits; i++) {
        auto vi = tree::make<values::ConstInt>(i);
        vi->copy_annotation<parser::SourceLocation>(*ast.num_qubits);
        all_qubits.add(vi);
    }
    scope.add("q", tree::make<values::QubitRefs>(all_qubits));
    scope.add("b", tree::make<values::BitRefs>(all_qubits));

    return result;
}

} // namespace analyzer
} // namespace cqasm
