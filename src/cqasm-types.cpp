#include "cqasm-types.hpp"

namespace cqasm {
namespace types {

/**
 * Type-checks and (if necessary) promotes the given value to the given type.
 * Returns null if the check/promotion fails, otherwise returns the constructed
 * value by way of a smart pointer.
 */
Value promote(const Value &value, const Type &type) {
#define return(type, value) return Value(new semantic::type(value))
    switch (type->type()) {
        case cqasm::semantic::NodeType::QubitType:
            if (value->type() == cqasm::semantic::NodeType::QubitRefs) {
                return(QubitRefs, *value->as_qubit_refs());
            }
            break;

        case cqasm::semantic::NodeType::BoolType:
            if (value->type() == cqasm::semantic::NodeType::BitRefs) {
                return(BitRefs, *value->as_bit_refs());
            } else if (value->type() == cqasm::semantic::NodeType::ConstBool) {
                return(ConstBool, *value->as_const_bool());
            }
            break;

        case cqasm::semantic::NodeType::AxisType:
            if (value->type() == cqasm::semantic::NodeType::ConstAxis) {
                return(ConstAxis, *value->as_const_axis());
            }
            break;

        case cqasm::semantic::NodeType::IntType:
            if (value->type() == cqasm::semantic::NodeType::ConstInt) {
                return(ConstInt, *value->as_const_int());
            }
            break;

        case cqasm::semantic::NodeType::RealType:
            if (value->type() == cqasm::semantic::NodeType::ConstInt) {
                return(ConstReal, cqasm::semantic::ConstReal(value->as_const_int()->value));
            } else if (value->type() == cqasm::semantic::NodeType::ConstReal) {
                return(ConstReal, *value->as_const_real());
            }
            break;

        case cqasm::semantic::NodeType::ComplexType:
            if (value->type() == cqasm::semantic::NodeType::ConstInt) {
                return(ConstComplex, cqasm::semantic::ConstComplex(value->as_const_int()->value));
            } else if (value->type() == cqasm::semantic::NodeType::ConstReal) {
                return(ConstComplex, cqasm::semantic::ConstComplex(value->as_const_real()->value));
            } else if (value->type() == cqasm::semantic::NodeType::ConstComplex) {
                return(ConstComplex, *value->as_const_complex());
            }
            break;

        case cqasm::semantic::NodeType::RealMatrixType: {
            auto mat_type = type->as_real_matrix_type();
            if (value->type() == cqasm::semantic::NodeType::ConstRealMatrix) {
                auto mat_value = value->as_const_real_matrix();
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if (mat_value->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if (mat_value->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
                        return(ConstRealMatrix, *mat_value);
                    }
                }
            }
            break;
        }

        case cqasm::semantic::NodeType::ComplexMatrixType: {
            auto mat_type = type->as_complex_matrix_type();
            if (value->type() == cqasm::semantic::NodeType::ComplexMatrixType) {
                const cqasm::semantic::ConstComplexMatrix *mat_value = value->as_const_complex_matrix();
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if (mat_value->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if (mat_value->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
                        return(ConstComplexMatrix, *mat_value);
                    }
                }
            } else if (value->type() == cqasm::semantic::NodeType::RealMatrixType) {
                auto mat_value = value->as_const_real_matrix();
                // Match matrix size. Negative sizes in the type mean unconstrained.
                if (mat_value->value.size_rows() == mat_type->num_rows || mat_type->num_rows < 0) {
                    if (mat_value->value.size_cols() == mat_type->num_cols || mat_type->num_cols < 0) {
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

        case cqasm::semantic::NodeType::StringType:
            if (value->type() == cqasm::semantic::NodeType::ConstString) {
                return(ConstString, *value->as_const_string());
            }
            break;

        case cqasm::semantic::NodeType::JsonType:
            if (value->type() == cqasm::semantic::NodeType::ConstJson) {
                return(ConstJson, *value->as_const_json());
            }
            break;
    }
#undef return

    // Can't promote.
    return Value();
}

/**
 * Represents a possible overload for the parameter types of a function, gate,
 * or error model. T is some tag type identifying the overload.
 */
template <class T>
class Overload {
private:
    T tag;
    Types param_types;
public:
    /**
     * Construct a possible overload.
     */
    template <class... Ts>
    Overload(const T &tag, const Types &param_types)
        : tag(tag), param_types(param_types)
    {}

    /**
     * Returns the tag for this overload.
     */
    const T &get_tag() const {
        return tag;
    }

    /**
     * Returns the number of parameters for this overload.
     */
    size_t num_params() const {
        return param_types.size();
    }

    /**
     * Returns the parameter type object for the parameter at the specified
     * index.
     */
    const Type &param_type_at(size_t index) const {
        return param_types.at(index);
    }
};

/**
 * Represents a set of possible overloads for the parameter types of a
 * function, gate, or error model. T is some tag type identifying the overload.
 * In case of a function, T would contain at least the return type, but maybe
 * also a lambda to represent the actual function. Note that ambiguous
 * overloads are silently resolved by using the first applicable overload, so
 * more specific overloads should always be added first.
 */
template <class T>
class OverloadResolver {
private:
    std::vector<Overload<T>> overloads;

public:

    /**
     * Adds a possible overload to the resolver. Note that ambiguous
     * overloads are silently resolved by using the first applicable overload,
     * so more specific overloads should always be added first.
     */
    void add_overload(const T &tag, const Types &param_types) {
        overloads.emplace_back(tag, param_types);
    }

    /**
     * Tries to resolve which overload belongs to the given argument list, if
     * any. Raises an OverloadResolutionFailure if no applicable overload
     * exists, otherwise the tag corresponding to the first proper overload and
     * the appropriately promoted vector of value pointers are returned.
     */
    std::pair<T, Values> resolve(const Values &args) {
        for (const auto &overload : overloads) {
            if (overload.num_params() != args.size()) {
                continue;
            }
            Values promoted_args;
            bool ok = true;
            for (size_t i = 0; i < args.size(); i++) {
                auto promoted_arg = promote(args.at(i), overload.param_type_at(i));
                if (!promoted_arg.empty()) {
                    ok = false;
                    break;
                }
                promoted_args.emplace_back(std::move(promoted_arg));
            }
            if (ok) {
                return std::pair<T, Values>(overload.get_tag(), promoted_args);
            }
        }
        throw OverloadResolutionFailure();
    }

};

/**
 * Table of overloaded callables with case-insensitive identifier matching. T
 * is the tag type of the callable/overload pair.
 */
template <class T>
class OverloadedNameResolver {
private:
    std::unordered_map<std::string, OverloadResolver<T>> table;
public:
    /**
     * Registers a callable. The name should be lowercase; matching will be done
     * case-insensitively. The param_types variadic specifies the amount and
     * types of the parameters that (this particular overload of) the function
     * expects. The C++ implementation of the function can assume that the
     * value list it gets is of the right size and the values are of the right
     * types. Note that ambiguous overloads are silently resolved by using the
     * first applicable overload, so more specific overloads should always be
     * added first.
     */
    void add_overload(const std::string &name, const T &tag, const Types &param_types) {
        std::string name_lower = name;
        std::for_each(name_lower.begin(), name_lower.end(), [](char &c){
            c = std::tolower(c);
        });
        auto entry = table.find(name_lower);
        if (entry == table.end()) {
            auto resolver = OverloadResolver<T>();
            resolver.add_overload(tag, param_types);
            table.insert(std::pair<std::string, OverloadResolver<T>>(name_lower, std::move(resolver)));
        } else {
            entry->second.add_overload(tag, param_types);
        }
    }

    /**
     * Resolves the particular overload for the callable with the given
     * case-insensitively matched name. Raises NameResolutionFailure if no
     * callable with the requested name is found, raises an
     * OverloadResolutionFailure if overload resolution fails, or otherwise
     * returns the tag of the first applicable callable/overload pair and the
     * appropriately promoted vector of value pointers.
     */
    std::pair<T, Values> resolve(const std::string &name, const Values &args) {
        std::string name_lower = name;
        std::for_each(name_lower.begin(), name_lower.end(), [](char &c){
            c = std::tolower(c);
        });
        auto entry = table.find(name_lower);
        if (entry == table.end()) {
            throw NameResolutionFailure();
        } else {
            return entry->second.resolve(args);
        }
    }

};

FunctionTable::FunctionTable() : resolver(new OverloadedNameResolver<FunctionImpl>()) {}

/**
 * Registers a function. The name should be lowercase; matching will be done
 * case-insensitively. The param_types variadic specifies the amount and
 * types of the parameters that (this particular overload of) the function
 * expects. The C++ implementation of the function can assume that the
 * value list it gets is of the right size and the values are of the right
 * types.
 */
void FunctionTable::add(const std::string &name, const FunctionImpl &impl, const Types &param_types) {
    resolver->add_overload(name, impl, param_types);
}

/**
 * Calls a function. Throws NameResolutionFailure if no function by the
 * given name exists, OverloadResolutionFailure if no overload of the
 * function exists for the given arguments, or otherwise returns the value
 * returned by the function.
 */
Value FunctionTable::call(const std::string &name, const Values &args) const {

    // Resolve the function and typecheck/promote the argument list.
    auto resolution = resolver->resolve(name, args);

    // Call the function with the typechecked/promoted argument list, and
    // return its result.
    return resolution.first(resolution.second);

}

/**
 * Creates a new instruction/error model type with the given name and
 * (optionally) supported parameter list. The parameter list is described
 * with one character per parameter, as follows:
 *  - q = qubit
 *  - a = axis (x, y, or z)
 *  - b = bit/boolean
 *  - i = integer
 *  - r = real
 *  - c = complex
 *  - u = complex matrix of size 4^n, where n is the number of qubits in
 *        the parameter list
 *  - s = (quoted) string
 *  - j = json
 * For more control, you can leave it empty (or unspecified) and manipulate
 * param_types manually.
 */
StatementType::StatementType(const std::string &name, const std::string &param_types) : name(name) {

    // Count the number of qubits, in case we find a unitary parameter.
    size_t num_qubits = 0;
    for (auto c : param_types) {
        if (c == 'q') {
            num_qubits += 1;
        }
    }

    // Now resolve the types.
    for (auto c : param_types) {
        switch (c) {
            case 'q':
                this->param_types.emplace_back(new semantic::QubitType());
                break;
            case 'a':
                this->param_types.emplace_back(new semantic::AxisType());
                break;
            case 'b':
                this->param_types.emplace_back(new semantic::BoolType());
                break;
            case 'i':
                this->param_types.emplace_back(new semantic::IntType());
                break;
            case 'r':
                this->param_types.emplace_back(new semantic::RealType());
                break;
            case 'c':
                this->param_types.emplace_back(new semantic::ComplexType());
                break;
            case 'u':
                this->param_types.emplace_back(new semantic::ComplexMatrixType(
                    1ull << num_qubits, 1ull << num_qubits));
                break;
            case 's':
                this->param_types.emplace_back(new semantic::StringType());
                break;
            case 'j':
                this->param_types.emplace_back(new semantic::JsonType());
                break;
            default:
                throw std::invalid_argument("unknown type code encountered");
        }
    }
}

ErrorModelTable::ErrorModelTable() : resolver(new OverloadedNameResolver<StatementType>()) {}

/**
 * Registers an error model.
 */
void ErrorModelTable::add(const StatementType &type) {
    resolver->add_overload(type.name, type, type.param_types);
}

/**
 * Resolves an error model. Throws NameResolutionFailure if no error model
 * by the given name exists, OverloadResolutionFailure if no overload
 * exists for the given arguments, or otherwise returns the resolved error
 * model node.
 */
semantic::ErrorModel ErrorModelTable::resolve(const std::string &name, const Values &args) const {
    // TODO
}

/**
 * Creates a new instruction type with the given name and (optionally)
 * supported parameter list. The parameter list is described with one
 * character per parameter, as follows:
 *  - q = qubit
 *  - a = axis (x, y, or z)
 *  - b = bit/boolean
 *  - i = integer
 *  - r = real
 *  - c = complex
 *  - u = complex matrix of size 4^n, where n is the number of qubits in
 *        the parameter list
 *  - s = (quoted) string
 *  - j = json
 * For more control, you can leave it empty (or unspecified) and manipulate
 * param_types manually.
 *
 * allow_conditional specifies whether the instruction can be made
 * conditional with c- notation. allow_parallel specifies whether it may
 * appear bundled with other instructions. allow_reused_qubits specifies
 * whether it is legal for the instruction to use a qubit more than once in
 * its parameter list.
 */
InstructionType::InstructionType(
    const std::string &name,
    const std::string &param_types,
    bool allow_conditional,
    bool allow_parallel,
    bool allow_reused_qubits
) :
    StatementType(name, param_types),
    allow_conditional(allow_conditional),
    allow_parallel(allow_parallel),
    allow_reused_qubits(allow_reused_qubits)
{};

InstructionTable::InstructionTable() : resolver(new OverloadedNameResolver<InstructionType>()) {}

/**
 * Registers an instruction type.
 */
void InstructionTable::add(const InstructionType &type) {
    resolver->add_overload(type.name, type, type.param_types);
}

/**
 * Resolves an instruction. Throws NameResolutionFailure if no instruction
 * by the given name exists, OverloadResolutionFailure if no overload
 * exists for the given arguments, or otherwise returns the resolved
 * instruction node.
 */
semantic::Instruction InstructionTable::resolve(const std::string &name, const Values &args) const {
    // TODO
}

} // namespace overload
} // namespace types
