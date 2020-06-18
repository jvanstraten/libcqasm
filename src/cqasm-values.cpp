#include <cqasm-parse-helper.hpp>
#include "cqasm-values.hpp"

namespace cqasm {
namespace values {

using TypeEnum = types::NodeType;
using ValueEnum = values::NodeType;

/**
 * Type-checks and (if necessary) promotes the given value to the given type.
 * Returns null if the check/promotion fails, otherwise returns the constructed
 * value by way of a smart pointer.
 */
Value promote(const Value &value, const types::Type &type) {
    Value retval;
    switch (type->type()) {
        case TypeEnum::Qubit:
            if (auto qubit_refs = value->as_qubit_refs()) {
                retval = tree::make<values::QubitRefs>(*qubit_refs);
            }
            break;

        case TypeEnum::Bool:
            if (auto bit_refs = value->as_bit_refs()) {
                retval = tree::make<values::BitRefs>(*bit_refs);
            } else if (auto const_bool = value->as_const_bool()) {
                retval = tree::make<values::ConstBool>(const_bool->value);
            }
            break;

        case TypeEnum::Axis:
            if (auto const_axis = value->as_const_axis()) {
                retval = tree::make<values::ConstAxis>(const_axis->value);
            }
            break;

        case TypeEnum::Int:
            if (auto const_int = value->as_const_int()) {
                retval = tree::make<values::ConstInt>(const_int->value);
            }
            break;

        case TypeEnum::Real:
            if (auto const_int = value->as_const_int()) {
                retval = tree::make<values::ConstReal>(const_int->value);
            } else if (auto const_real = value->as_const_real()) {
                retval = tree::make<values::ConstReal>(const_real->value);
            }
            break;

        case TypeEnum::Complex:
            if (auto const_int = value->as_const_int()) {
                retval = tree::make<values::ConstComplex>(const_int->value);
            } else if (auto const_real = value->as_const_real()) {
                retval = tree::make<values::ConstComplex>(const_real->value);
            } else if (auto const_complex = value->as_const_complex()) {
                retval = tree::make<values::ConstComplex>(const_complex->value);
            }
            break;

        case TypeEnum::RealMatrix: {
            auto mat_type = type->as_real_matrix();
            if (auto const_real_matrix = value->as_const_real_matrix()) {
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if ((ssize_t)const_real_matrix->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if ((ssize_t)const_real_matrix->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
                        retval = tree::make<values::ConstRealMatrix>(const_real_matrix->value);
                    }
                }
            }
            break;
        }

        case TypeEnum::ComplexMatrix: {
            auto mat_type = type->as_complex_matrix();
            if (auto const_complex_matrix = value->as_const_complex_matrix()) {
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if ((ssize_t)const_complex_matrix->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if ((ssize_t)const_complex_matrix->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
                        retval = tree::make<values::ConstComplexMatrix>(const_complex_matrix->value);
                    }
                }
            } else if (auto const_real_matrix = value->as_const_real_matrix()) {
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if ((ssize_t)const_real_matrix->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if ((ssize_t)const_real_matrix->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
                        // Convert double to complex.
                        const size_t rows = const_real_matrix->value.size_rows();
                        const size_t cols = const_real_matrix->value.size_cols();
                        cqasm::primitives::CMatrix complex_mat_value(rows, cols);
                        for (size_t row = 1; row <= rows; row++) {
                            for (size_t col = 1; col <= cols; col++) {
                                complex_mat_value.at(row, col) = const_real_matrix->value.at(row, col);
                            }
                        }
                        retval = tree::make<values::ConstComplexMatrix>(complex_mat_value);
                        break;
                    }
                }
                // NOTE: DEPRECATED BEHAVIOR, FOR BACKWARDS COMPATIBILITY ONLY
                // If the expected matrix has a defined size and is square, and
                // the real matrix is a vector with the 2 * 4**n entries, we
                // interpret it as an old-style cqasm unitary matrix, from
                // before cqasm knew what complex numbers (or multidimensional
                // matrices) were.
                if (mat_type->num_rows == mat_type->num_cols && mat_type->num_rows > 0) {
                    const size_t size = mat_type->num_rows;
                    const size_t num_elements = 2ull << (2 * size);
                    if (const_real_matrix->value.size_rows() == 1 && const_real_matrix->value.size_cols() == num_elements) {
                        cqasm::primitives::CMatrix complex_mat_value(size, size);
                        size_t index = 0;
                        for (size_t row = 1; row <= size; row++) {
                            for (size_t col = 1; col <= size; col++) {
                                double re = const_real_matrix->value.at(1, index++);
                                double im = const_real_matrix->value.at(1, index++);
                                complex_mat_value.at(row, col) = std::complex<double>(re, im);
                            }
                        }
                        retval = tree::make<values::ConstComplexMatrix>(complex_mat_value);
                    }
                }
            }
            break;
        }

        case TypeEnum::String:
            if (auto const_string = value->as_const_string()) {
                retval = tree::make<values::ConstString>(const_string->value);
            }
            break;

        case TypeEnum::Json:
            if (auto const_json = value->as_const_json()) {
                retval = tree::make<values::ConstJson>(const_json->value);
            }
            break;
    }

    // Copy source location annotations into the new object.
    if (retval) {
        retval->copy_annotation<parser::SourceLocation>(*value);
    }

    // Can't promote.
    return Value();
}

} // namespace values
} // namespace cqasm

/**
 * Stream << overload for a single value.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::values::Value& value) {
    if (value.empty()) {
        os << "NULL";
    } else {
        os << *value;
    }
    return os;
}

/**
 * Stream << overload for zero or more values.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::values::Values& values) {
    os << "[";
    bool first = true;
    for (const auto &value : values) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        if (value) {
            os << *value;
        } else {
            os << "NULL";
        }
    }
    os << "]";
    return os;
}
