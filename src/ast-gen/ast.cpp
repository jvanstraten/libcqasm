#include "ast-gen.hpp"

/**
 * Constructs the cQASM AST nodes.
 */
std::vector<std::shared_ptr<NodeType>> build_nodes() {

    // The set of all nodes is gathered in this vector.
    auto nodes = std::vector<std::shared_ptr<NodeType>>();

    //=========================================================================
    // Expressions
    //=========================================================================
    auto expression =
        NodeBuilder(
            "expression",
            "Any kind of expression.")
        .build(nodes);

    auto expression_list =
        NodeBuilder(
            "expression_list",
            "Represents a comma-separated list of expressions.")
        .with(Any, expression, "items", "The list of expressions,")
        .build(nodes);

    auto erroneous_expression =
        NodeBuilder(
            "erroneous_expression",
            "Placeholder for an expression with a parse error.")
        .derive_from(expression)
        .marks_error()
        .build(nodes);

    //-------------------------------------------------------------------------
    // Simple literals 
    //-------------------------------------------------------------------------
    auto integer_literal =
        NodeBuilder(
            "integer_literal",
            "An integer literal.")
        .derive_from(expression)
        .with(Int, "value", "The integer.")
        .build(nodes);

    auto float_literal =
        NodeBuilder(
            "float_literal",
            "A floating point literal.")
        .derive_from(expression)
        .with(Real, "value", "The floating point number.")
        .build(nodes);

    auto identifier =
        NodeBuilder(
            "identifier",
            "An identifier.")
        .derive_from(expression)
        .with(Str, "name", "The identifier.")
        .build(nodes);

    //-------------------------------------------------------------------------
    // Matrix literals 
    //-------------------------------------------------------------------------
    auto matrix_literal =
        NodeBuilder(
            "matrix_literal",
            "Represents a matrix literal.")
        .derive_from(expression)
        .build(nodes);

    auto matrix_literal_1 =
        NodeBuilder(
            "matrix_literal_1",
            "Represents a square matrix literal represented as a flattened "
            "list of row-major real/imaginary expression pairs.")
        .derive_from(matrix_literal)
        .with(
            One, expression_list, "pairs",
            "The list of row-major real/imaginary expression pairs.")
        .build(nodes);

    auto matrix_literal_2 =
        NodeBuilder(
            "matrix_literal_2",
            "Represents a matrix literal represented as a list of rows, "
            "which are in turn lists of complex expressions.")
        .derive_from(matrix_literal)
        .with(
            Many, expression_list, "rows",
            "The list of rows in the matrix.")
        .build(nodes);

    //-------------------------------------------------------------------------
    // String literals
    //-------------------------------------------------------------------------
    auto string_literal =
        NodeBuilder(
            "string_literal",
            "Represents a string literal.")
        .derive_from(expression)
        .with(Str, "value", "The string literal.")
        .build(nodes);

    auto json_literal =
        NodeBuilder(
            "json_literal",
            "Represents a JSON literal.")
        .derive_from(expression)
        .with(Str, "value", "The JSON literal.")
        .build(nodes);

    //-------------------------------------------------------------------------
    // Function calls
    //-------------------------------------------------------------------------
    auto function_call =
        NodeBuilder(
            "function_call",
            "A function call.")
        .derive_from(expression)
        .with(One, identifier, "name", "The name of the function.")
        .with(One, expression_list, "arguments", "The function arguments.")
        .build(nodes);

    //-------------------------------------------------------------------------
    // Indexation operator
    //-------------------------------------------------------------------------
    auto index_entry =
        NodeBuilder(
            "index_entry",
            "An entry in an index list. Can be a single index or a range.")
        .build(nodes);

    auto index_item =
        NodeBuilder(
            "index_item",
            "A single index in an index list.")
        .derive_from(index_entry)
        .with(One, expression, "index", "An integer expression representing the index.")
        .build(nodes);

    auto index_range =
        NodeBuilder(
            "index_range",
            "An inclusive range of indices in an index list.")
        .derive_from(index_entry)
        .with(One, expression, "first", "An integer expression representing the first index.")
        .with(One, expression, "last", "An integer expression representing the last index.")
        .build(nodes);

    auto index_list =
        NodeBuilder(
            "index_list",
            "A list of one or more indices.")
        .with(Many, index_entry, "items", "The list of indices.")
        .build(nodes);

    auto index =
        NodeBuilder(
            "index",
            "An indexation expression.")
        .derive_from(expression)
        .with(One, expression, "expr", "The expression being indexed.")
        .with(One, index_list, "indices", "The list of indices.")
        .build(nodes);

    //-------------------------------------------------------------------------
    // Unary operators
    //-------------------------------------------------------------------------
    auto unary_op =
        NodeBuilder(
            "unary_op",
            "Any unary operator.")
        .derive_from(expression)
        .with(One, expression, "expr", "The expression being operated on.")
        .build(nodes);

    auto negate =
        NodeBuilder(
            "negate",
            "Negation operator.")
        .derive_from(unary_op)
        .build(nodes);

    //-------------------------------------------------------------------------
    // Binary operators
    //-------------------------------------------------------------------------
    auto binary_op =
        NodeBuilder(
            "binary_op",
            "Any binary operator.")
        .derive_from(expression)
        .with(One, expression, "lhs", "The left-hand side of the expression.")
        .with(One, expression, "rhs", "The right-hand side of the expression.")
        .build(nodes);

    auto power =
        NodeBuilder(
            "power",
            "Power operator.")
        .derive_from(binary_op)
        .build(nodes);

    auto multiply =
        NodeBuilder(
            "multiply",
            "Multiplication operator.")
        .derive_from(binary_op)
        .build(nodes);

    auto divide =
        NodeBuilder(
            "divide",
            "Division operator.")
        .derive_from(binary_op)
        .build(nodes);

    auto add =
        NodeBuilder(
            "add",
            "Addition operator.")
        .derive_from(binary_op)
        .build(nodes);

    auto subtract =
        NodeBuilder(
            "subtract",
            "Subtraction operator.")
        .derive_from(binary_op)
        .build(nodes);

    //=========================================================================
    // Annotations
    //=========================================================================
    auto annotation_data =
        NodeBuilder(
            "annotation_data",
            "Represents an annotation.")
        .with(
            One, identifier, "interface",
            "The interface this annotation is intended for. If a target "
            "doesn't support an interface, it should silently ignore the "
            "annotation.")
        .with(
            One, identifier, "operation",
            "The operation within the interface that this annotation is "
            "intended for. If a supports the corresponding interface but not "
            "the operation, it should throw an error.")
        .with(
            Maybe, expression_list, "operands",
            "Any operands attached to the annotation.")
        .build(nodes);

    auto annotated =
        NodeBuilder(
            "annotated",
            "Represents a node that carries annotation data.")
        .with(
            Any, annotation_data, "annotations",
            "Zero or more annotations attached to this object.")
        .build(nodes);

    //=========================================================================
    // Statements
    //=========================================================================
    auto statement =
        NodeBuilder(
            "statement",
            "Any kind of statement.")
        .derive_from(annotated)
        .build(nodes);

    auto statement_list =
        NodeBuilder(
            "statement_list",
            "Any kind of statement.")
        .with(Any, statement, "items", "The list of statements.")
        .build(nodes);

    auto erroneous_statement =
        NodeBuilder(
            "erroneous_statement",
            "Placeholder for a statement with a parse error.")
        .derive_from(statement)
        .marks_error()
        .build(nodes);

    //-------------------------------------------------------------------------
    // Instructions
    //-------------------------------------------------------------------------
    auto instruction =
        NodeBuilder(
            "instruction",
            "Any kind of instruction. Note that this is NOT a statement; "
            "instructions are always considered part of a parallel bundle, "
            "even if they're on their own.")
        .derive_from(annotated)
        .with(
            One, identifier, "name",
            "Name identifying the instruction.")
        .with(
            Maybe, expression, "condition",
            "Optional conditional expression.")
        .with(
            One, expression_list, "operands",
            "Operands for the instruction.")
        .build(nodes);

    auto bundle =
        NodeBuilder(
            "bundle",
            "A list of parallel instructions.")
        .derive_from(statement)
        .with(Many, instruction, "items", "The list of parallel instructions.")
        .build(nodes);

    //-------------------------------------------------------------------------
    // Mappings
    //-------------------------------------------------------------------------
    auto mapping =
        NodeBuilder(
            "mapping",
            "A mapping (alias) for an expression. Originally just a way of "
            "naming a single qubit.")
        .derive_from(statement)
        .with(
            One, identifier, "alias",
            "The identifier used to refer to the expression.")
        .with(
            One, expression, "expr",
            "The aliased expression.")
        .build(nodes);

    //-------------------------------------------------------------------------
    // Subcircuits
    //-------------------------------------------------------------------------
    auto subcircuit =
        NodeBuilder(
            "subcircuit",
            "A subcircuit header.")
        .derive_from(statement)
        .with(One, identifier, "name", "The name of the subcircuit.")
        .with(
            Maybe, expression, "iterations",
            "An optional integer expression representing the number of "
            "iterations for this subcircuit.")
        .build(nodes);

    //=========================================================================
    // Program structure
    //=========================================================================
    auto version =
        NodeBuilder(
            "version",
            "The file version identifier.")
        .with(
            Version, "items",
            "The list of version components, ordered major to minor.")
        .build(nodes);

    auto root =
        NodeBuilder(
            "root",
            "Any root node for the AST.")
        .build(nodes);

    auto erroneous_program =
        NodeBuilder(
            "erroneous_program",
            "Placeholder for a program with a parse error.")
        .derive_from(root)
        .marks_error()
        .build(nodes);

    auto program =
        NodeBuilder(
            "program",
            "A comlete program.")
        .with(
            One, version, "version",
            "File file version.")
        .with(
            One, expression, "num_qubits",
            "Integer expression indicating the number of qubits.")
        .with(
            One, statement_list, "statements",
            "The statement list.")
        .derive_from(root)
        .build(nodes);

    return nodes;
}
