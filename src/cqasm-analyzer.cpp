#include <cqasm-parse-helper.hpp>
#include "cqasm-analyzer.hpp"

namespace cqasm {
namespace analyzer {

/**
 * Scope information.
 */
class Scope {
public:
    resolver::MappingTable mappings;
    resolver::FunctionTable functions;
    resolver::InstructionTable instruction_set;

    Scope(
        const resolver::MappingTable &mappings,
        const resolver::FunctionTable &functions,
        const resolver::InstructionTable &instruction_set
    ) :
        mappings(mappings),
        functions(functions),
        instruction_set(instruction_set)
    {}

};

/**
 * Helper class for analyzing a single AST. This contains the stateful
 * information that Analyzer can't have (to allow Analyzer to be reused).
 */
class AnalyzerHelper {
public:
    const Analyzer &analyzer;
    AnalysisResult result;
    Scope scope;

    /**
     * Analyzes the given AST using the given analyzer.
     */
    AnalyzerHelper(const Analyzer &analyzer, const ast::Program &ast);

    /**
     * Parses the version tag. Any semantic errors encountered are pushed into
     * the result error vector.
     */
    void analyze_version(const ast::Version &ast);

    /**
     * Checks the qubits statement and updates the scope accordingly. Any
     * semantic errors encountered are pushed into the result error vector.
     */
    void analyze_qubits(const ast::Expression &count);

    /**
     * Parses any kind of expression. Always returns a filled value or throws
     * an exception.
     */
    values::Value parse_expression(const ast::Expression &expression);

    /**
     * Shorthand for parsing an expression and promoting it to the given type,
     * constructed in-place with the type_args parameter pack. Returns empty
     * when the cast fails.
     */
    template<class Type, class... TypeArgs>
    values::Value parse_expression_as(
        const ast::Expression &expression,
        TypeArgs... type_args
    );

    /**
     * Shorthand for parsing an expression to a constant integer.
     */
    primitives::Int parse_const_int(const ast::Expression &expression);

    /**
     * Parses a matrix. Always returns a filled value or throws an exception.
     */
    values::Value parse_matrix(const ast::MatrixLiteral &matrix_lit);

    /**
     * Helper for parsing a matrix. Highly templated to avoid repeating the
     * same code for different kinds of matrices, but bear in mind that the
     * template parameters are codependent. Returns empty on failure.
     */
    template<class ElType, class ElVal, class MatLit, class MatVal>
    values::Value parse_matrix_helper(
        size_t nrows, size_t ncols,
        const std::vector<values::Value> &vals
    );

    /**
     * Parses an index operator. Always returns a filled value or throws an
     * error.
     */
    values::Value parse_index(const ast::Index &index);

    /**
     * Parses an index list.
     */
    tree::Many<values::ConstInt>
    parse_index_list(const ast::IndexList &index_list, size_t size);

    /**
     * Parses a function. Always returns a filled value or throws an exception.
     */
    values::Value parse_function(
        const ast::Identifier &name,
        const ast::ExpressionList &args
    );

    /**
     * Parses an operator. Always returns a filled value or throws an exception.
     */
    values::Value parse_operator(
        const std::string &name,
        const tree::One<ast::Expression> &a,
        const tree::One<ast::Expression> &b = tree::One<ast::Expression>()
    );

};

/**
 * Analyzes the given AST.
 */
AnalysisResult Analyzer::analyze(const ast::Program &ast) const {
    return AnalyzerHelper(*this, ast).result;
}

/**
 * Analyzes the given AST using the given analyzer.
 */
AnalyzerHelper::AnalyzerHelper(
    const Analyzer &analyzer,
    const ast::Program &ast
) :
    analyzer(analyzer),
    result(),
    scope(analyzer.mappings, analyzer.functions, analyzer.instruction_set)
{
    try {

        // Construct the program node.
        result.root.set(tree::make<semantic::Program>());
        result.root->copy_annotation<parser::SourceLocation>(ast);

        // Check and set the version.
        analyze_version(*ast.version);

        // Handle the qubits statement.
        analyze_qubits(*ast.num_qubits);

        // TODO

    } catch (error::AnalysisError &e) {
        result.errors.push_back(e.get_message());
    }
}

/**
 * Checks the AST version node and puts it into the semantic tree.
 */
void AnalyzerHelper::analyze_version(const ast::Version &ast) {
    try {
        result.root->version = tree::make<semantic::Version>();
        for (auto item : ast.items) {
            if (item < 0) {
                throw error::AnalysisError("invalid version component");
            }
        }
        result.root->version->items = ast.items;
    } catch (error::AnalysisError &e) {
        e.context(ast);
        result.errors.push_back(e.get_message());
    }
    result.root->version->copy_annotation<parser::SourceLocation>(ast);
}

/**
 * Checks the qubits statement and updates the scope accordingly. Any
 * semantic errors encountered are pushed into the result error vector.
 */
void AnalyzerHelper::analyze_qubits(const ast::Expression &count) {
    try {
        // Default to 0 qubits in case we get an exception.
        result.root->num_qubits = 0;

        // Try to load the number of qubits from the expression.
        result.root->num_qubits = parse_const_int(count);
        if (result.root->num_qubits < 1) {
            // Number of qubits must be positive.
            throw error::AnalysisError("invalid number of qubits");
        }

        // Construct the special q and b mappings, that map to the whole qubit
        // and measurement register respectively.
        tree::Many<values::ConstInt> all_qubits;
        for (primitives::Int i = 0; i < result.root->num_qubits; i++) {
            auto vi = tree::make<values::ConstInt>(i);
            vi->copy_annotation<parser::SourceLocation>(count);
            all_qubits.add(vi);
        }
        scope.mappings.add("q", tree::make<values::QubitRefs>(all_qubits));
        scope.mappings.add("b", tree::make<values::BitRefs>(all_qubits));

    } catch (error::AnalysisError &e) {
        e.context(count);
        result.errors.push_back(e.get_message());
    }
}

/**
 * Parses any kind of expression. Always returns a filled value or throws
 * an exception.
 */
values::Value AnalyzerHelper::parse_expression(const ast::Expression &expression) {
    values::Value retval;
    try {
        if (auto int_lit = expression.as_integer_literal()) {
            retval.set(tree::make<values::ConstInt>(int_lit->value));
        } else if (auto float_lit = expression.as_float_literal()) {
            retval.set(tree::make<values::ConstReal>(float_lit->value));
        } else if (auto string_lit = expression.as_string_literal()) {
            retval.set(tree::make<values::ConstString>(string_lit->value));
        } else if (auto json_lit = expression.as_json_literal()) {
            retval.set(tree::make<values::ConstJson>(json_lit->value));
        } else if (auto matrix_lit = expression.as_matrix_literal()) {
            retval.set(parse_matrix(*matrix_lit));
        } else if (auto ident = expression.as_identifier()) {
            retval.set(scope.mappings.resolve(ident->name));
        } else if (auto index = expression.as_index()) {
            retval.set(parse_index(*index));
        } else if (auto func = expression.as_function_call()) {
            retval.set(parse_function(func->name->name, *func->arguments));
        } else if (auto negate = expression.as_negate()) {
            retval.set(parse_operator("-", negate->expr));
        } else if (auto power = expression.as_power()) {
            retval.set(parse_operator("**", power->lhs, power->rhs));
        } else if (auto mult = expression.as_multiply()) {
            retval.set(parse_operator("*", mult->lhs, mult->rhs));
        } else if (auto div = expression.as_divide()) {
            retval.set(parse_operator("/", div->lhs, div->rhs));
        } else if (auto add = expression.as_add()) {
            retval.set(parse_operator("+", add->lhs, add->rhs));
        } else if (auto sub = expression.as_subtract()) {
            retval.set(parse_operator("-", sub->lhs, sub->rhs));
        } else {
            throw std::runtime_error("unexpected expression node");
        }
    } catch (error::AnalysisError &e) {
        e.context(expression);
        throw;
    }
    if (!retval) {
        throw std::runtime_error(
            "parse_expression returned nonsense, this should never happen");
    }
    retval->copy_annotation<parser::SourceLocation>(expression);
    return retval;
}

/**
 * Shorthand for parsing an expression and promoting it to the given type,
 * constructed in-place with the type_args parameter pack. Returns empty
 * when the cast fails.
 */
template <class Type, class... TypeArgs>
values::Value AnalyzerHelper::parse_expression_as(const ast::Expression &expression, TypeArgs... type_args) {
    return values::promote(parse_expression(expression), tree::make<Type>(type_args...));
}

/**
 * Shorthand for parsing an expression to a constant integer.
 */
primitives::Int AnalyzerHelper::parse_const_int(const ast::Expression &expression) {
    auto value = parse_expression_as<types::Int>(expression);
    if (auto int_value = value->as_const_int()) {
        return int_value->value;
    } else {
        throw error::AnalysisError("constant integer expected");
    }
}

/**
 * Parses a matrix. Always returns a filled value or throws an exception.
 */
values::Value AnalyzerHelper::parse_matrix(const ast::MatrixLiteral &matrix_lit) {

    // Figure out the size of the matrix and parse the subexpressions.
    // Note that the number of rows is always at least 1 (Many vs Any) so
    // the ncols line is well-behaved.
    size_t nrows = matrix_lit.rows.size();
    size_t ncols = matrix_lit.rows[0].size();
    std::vector<values::Value> vals;
    for (size_t row = 0; row < nrows; row++) {
        for (size_t col = 0; col < ncols; col++) {
            vals.push_back(parse_expression(*matrix_lit.rows[row]->items[col]));
        }
    }

    // Try building a matrix of constant real numbers.
    auto value = parse_matrix_helper<
        types::Real, values::ConstReal,
        primitives::RMatrix, values::ConstRealMatrix
    >(nrows, ncols, vals);
    if (value) {
        return value;
    }

    // Try building a matrix of constant complex numbers.
    value = parse_matrix_helper<
        types::Complex, values::ConstComplex,
        primitives::CMatrix, values::ConstComplexMatrix
    >(nrows, ncols, vals);
    if (value) {
        return value;
    }

    // Only real and complex are supported right now. If more is to be
    // added in the future, this should probably be written a little
    // neater.
    throw error::AnalysisError("only matrices of constant real or complex numbers are currently supported");

}

/**
 * Helper for parsing a matrix. Highly templated to avoid repeating the
 * same code for different kinds of matrices, but bear in mind that the
 * template parameters are codependent. Returns empty on failure.
 */
template <class ElType, class ElVal, class MatLit, class MatVal>
values::Value AnalyzerHelper::parse_matrix_helper(
    size_t nrows, size_t ncols,
    const std::vector<values::Value> &vals
) {
    auto matrix = MatLit(nrows, ncols);
    for (size_t row = 0; row < nrows; row++) {
        for (size_t col = 0; col < ncols; col++) {
            if (auto val = values::promote(vals[row * ncols + col], tree::make<ElType>())) {
                if (auto val_real = val.template as<ElVal>()) {
                    matrix.at(row + 1, col + 1) = val_real->value;
                } else {
                    return values::Value();
                }
            }
        }
    }
    return tree::make<MatVal>(matrix);
}

/**
 * Parses an index operator. Always returns a filled value or throws an error.
 */
values::Value AnalyzerHelper::parse_index(const ast::Index &index) {
    auto expr = parse_expression(*index.expr);
    if (auto qubit_refs = expr->as_qubit_refs()) {

        // Qubit refs.
        auto indices = parse_index_list(*index.indices, qubit_refs->index.size());
        for (auto idx : indices) {
            idx->value = qubit_refs->index[idx->value]->value;
        }
        return tree::make<values::QubitRefs>(indices);

    } else if (auto bit_refs = expr->as_bit_refs()) {

        // Measurement bit refs.
        auto indices = parse_index_list(*index.indices, bit_refs->index.size());
        for (auto idx : indices) {
            idx->value = bit_refs->index[idx->value]->value;
        }
        return tree::make<values::BitRefs>(indices);

    } else {

        // While matrices could conceivably be indexed, this is not supported
        // right now.
        std::ostringstream ss;
        ss << "indexation is not supported for value of type " << values::type_of(expr);
        throw error::AnalysisError(ss.str());

    }
}

/**
 * Parses an index list.
 */
tree::Many<values::ConstInt> AnalyzerHelper::parse_index_list(const ast::IndexList &index_list, size_t size) {
    tree::Many<values::ConstInt> retval;
    for (auto entry : index_list.items) {
        if (auto item = entry->as_index_item()) {

            // Single index.
            auto index = parse_const_int(*item->index);
            if (index < 0 || (unsigned long)index >= size) {
                throw error::AnalysisError(
                    "index " + std::to_string(index)
                    + " out of range (size " + std::to_string(size) + ")",
                    item);
            }
            auto index_val = tree::make<values::ConstInt>(index);
            index_val->copy_annotation<parser::SourceLocation>(*item);
            retval.add(index_val);

        } else if (auto range = entry->as_index_range()) {

            // Range notation.
            auto first = parse_const_int(*range->first);
            if (first < 0 || (unsigned long)first >= size) {
                throw error::AnalysisError(
                    "index " + std::to_string(first)
                    + " out of range (size " + std::to_string(size) + ")",
                    &*range->first);
            }
            auto last = parse_const_int(*range->last);
            if (last < 0 || (unsigned long)last >= size) {
                throw error::AnalysisError(
                    "index " + std::to_string(last)
                    + " out of range (size " + std::to_string(size) + ")",
                    &*range->first);
            }
            if (first > last) {
                throw error::AnalysisError("last index is lower than first index", range);
            }
            for (auto index = (size_t)first; index <= (size_t)last; index++) {
                auto index_val = tree::make<values::ConstInt>(index);
                index_val->copy_annotation<parser::SourceLocation>(*range);
                retval.add(index_val);
            }

        } else {
            throw std::runtime_error("unknown IndexEntry AST node");
        }
    }
    return retval;
}

/**
 * Parses a function. Always returns a filled value or throws an exception.
 */
values::Value AnalyzerHelper::parse_function(const ast::Identifier &name, const ast::ExpressionList &args) {
    auto arg_values = values::Values();
    for (auto arg : args.items) {
        arg_values.add(parse_expression(*arg));
    }
    auto retval = scope.functions.call(name.name, arg_values);
    if (!retval) {
        throw std::runtime_error("function implementation returned empty value");
    }
    return retval;
}

/**
 * Parses an operator. Always returns a filled value or throws an exception.
 */
values::Value AnalyzerHelper::parse_operator(
    const std::string &name,
    const tree::One<ast::Expression> &a,
    const tree::One<ast::Expression> &b
) {
    auto identifier = ast::Identifier("operator" + name);
    auto args = ast::ExpressionList();
    args.items.add(a);
    args.items.add(b);
    return parse_function(identifier, args);
}

} // namespace analyzer
} // namespace cqasm
