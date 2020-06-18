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
#define return(type, value) return Value(new values::type(value))
    switch (type->type()) {
        case TypeEnum::Qubit:
            if (value->type() == ValueEnum::QubitRefs) {
                return(QubitRefs, *value->as_qubit_refs());
            }
            break;

        case TypeEnum::Bool:
            if (value->type() == ValueEnum::BitRefs) {
                return(BitRefs, *value->as_bit_refs());
            } else if (value->type() == ValueEnum::ConstBool) {
                return(ConstBool, *value->as_const_bool());
            }
            break;

        case TypeEnum::Axis:
            if (value->type() == ValueEnum::ConstAxis) {
                return(ConstAxis, *value->as_const_axis());
            }
            break;

        case TypeEnum::Int:
            if (value->type() == ValueEnum::ConstInt) {
                return(ConstInt, *value->as_const_int());
            }
            break;

        case TypeEnum::Real:
            if (value->type() == ValueEnum::ConstInt) {
                return(ConstReal, values::ConstReal(value->as_const_int()->value));
            } else if (value->type() == ValueEnum::ConstReal) {
                return(ConstReal, *value->as_const_real());
            }
            break;

        case TypeEnum::Complex:
            if (value->type() == ValueEnum::ConstInt) {
                return(ConstComplex, values::ConstComplex(value->as_const_int()->value));
            } else if (value->type() == ValueEnum::ConstReal) {
                return(ConstComplex, values::ConstComplex(value->as_const_real()->value));
            } else if (value->type() == ValueEnum::ConstComplex) {
                return(ConstComplex, *value->as_const_complex());
            }
            break;

        case TypeEnum::RealMatrix: {
            auto mat_type = type->as_real_matrix();
            if (value->type() == ValueEnum::ConstRealMatrix) {
                auto mat_value = value->as_const_real_matrix();
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if ((ssize_t)mat_value->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if ((ssize_t)mat_value->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
                        return(ConstRealMatrix, *mat_value);
                    }
                }
            }
            break;
        }

        case TypeEnum::ComplexMatrix: {
            auto mat_type = type->as_complex_matrix();
            if (value->type() == ValueEnum::ConstComplexMatrix) {
                const values::ConstComplexMatrix *mat_value = value->as_const_complex_matrix();
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if ((ssize_t)mat_value->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if ((ssize_t)mat_value->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
                        return(ConstComplexMatrix, *mat_value);
                    }
                }
            } else if (value->type() == ValueEnum::ConstRealMatrix) {
                auto mat_value = value->as_const_real_matrix();
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if ((ssize_t)mat_value->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if ((ssize_t)mat_value->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
                        // Convert double to complex.
                        const size_t rows = mat_value->value.size_rows();
                        const size_t cols = mat_value->value.size_cols();
                        cqasm::primitives::CMatrix complex_mat_value(rows, cols);
                        for (size_t row = 1; row <= rows; row++) {
                            for (size_t col = 1; col <= cols; col++) {
                                complex_mat_value.at(row, col) = mat_value->value.at(row, col);
                            }
                        }
                        return(ConstComplexMatrix, complex_mat_value);
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
                    if (mat_value->value.size_rows() == 1 && mat_value->value.size_cols() == num_elements) {
                        cqasm::primitives::CMatrix complex_mat_value(size, size);
                        size_t index = 0;
                        for (size_t row = 1; row <= size; row++) {
                            for (size_t col = 1; col <= size; col++) {
                                double re = mat_value->value.at(1, index++);
                                double im = mat_value->value.at(1, index++);
                                complex_mat_value.at(row, col) = std::complex<double>(re, im);
                            }
                        }
                        return(ConstComplexMatrix, complex_mat_value);
                    }
                }
            }
            break;
        }

        case TypeEnum::String:
            if (value->type() == ValueEnum::ConstString) {
                return(ConstString, *value->as_const_string());
            }
            break;

        case TypeEnum::Json:
            if (value->type() == ValueEnum::ConstJson) {
                return(ConstJson, *value->as_const_json());
            }
            break;
    }
#undef return

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
        os << value.get();
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
