#ifndef _CQASM_TYPES_INCLUDED_
#define _CQASM_TYPES_INCLUDED_

#include <functional>
#include <algorithm>
#include "cqasm-semantic.hpp"

namespace cqasm {
namespace types {

/**
 * Shared pointer to a value of a dynamic type.
 */
using Value = tree::One<semantic::Value>;

/**
 * Shared pointer to a number of values of dynamic types.
 */
using Values = std::vector<Value>;

/**
 * Shared pointer to a type object of a dynamic type.
 */
using Type = tree::One<semantic::Type>;

/**
 * Shared pointer to a number of type objects of dynamic types.
 */
using Types = std::vector<Type>;

/**
 * Type-checks and (if necessary) promotes the given value to the given type.
 * Returns null if the check/promotion fails, otherwise returns the constructed
 * value by way of a smart pointer.
 */
Value promote(const Value &value, const Type &type);

/**
 * Exception for failed name resolutions.
 */
class NameResolutionFailure : std::exception {
public:
    const char *what() const noexcept override {
        return "use of undefined name";
    }
};

/**
 * Exception for failed overload resolutions.
 */
class OverloadResolutionFailure : std::exception {
public:
    const char *what() const noexcept override {
        return "overload resolution failure";
    }
};

// Forward declaration for the name resolver template class. This class is
// defined entirely in the C++ file to cut back on compile time.
template <class T>
class OverloadedNameResolver;

/**
 * C++ function representing (one of the overloads of) a function usable in
 * cQASM constant expressions.
 */
using FunctionImpl = std::function<Value(const Values&)>;

/**
 * Table of all overloads of all constant propagation functions.
 */
class FunctionTable {
private:
    std::unique_ptr<OverloadedNameResolver<FunctionImpl>> resolver;
public:

    FunctionTable();

    /**
     * Registers a function. The name should be lowercase; matching will be done
     * case-insensitively. The param_types variadic specifies the amount and
     * types of the parameters that (this particular overload of) the function
     * expects. The C++ implementation of the function can assume that the
     * value list it gets is of the right size and the values are of the right
     * types.
     */
    void add(const std::string &name, const FunctionImpl &impl, const Types &param_types);

    /**
     * Calls a function. Throws NameResolutionFailure if no function by the
     * given name exists, OverloadResolutionFailure if no overload of the
     * function exists for the given arguments, or otherwise returns the value
     * returned by the function.
     */
    Value call(const std::string &name, const Values &args) const;

};

/**
 * Representation of an instruction (aka gate) or error model type.
 * InstructionTable and ErrorModelTable contain a map of these, used by libqasm
 * to match the instructions/error models it finds in the cQASM file with the
 * supported instruction/error model set.
 *
 * As a user of the instruction resolution functionality, you may want to add
 * your own data to the InstructionTypes that you register. For instance, a
 * simulator may want to attach the gate matrix. This can be done using
 * annotations. Refer to cqasm-annotatable.hpp for more info.
 */
class StatementType : public annotatable::Annotatable {
public:
    /**
     * The name of the instruction or error model. Note that names are matched
     * case insensitively.
     */
    std::string name;

    /**
     * The vector of parameter types that this instruction/error model expects.
     */
    Types param_types;

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
    StatementType(const std::string &name, const std::string &param_types = "");

};

/**
 * Table of the supported instructions and their overloads.
 */
class ErrorModelTable {
private:
    std::unique_ptr<OverloadedNameResolver<StatementType>> resolver;
public:

    ErrorModelTable();

    /**
     * Registers an error model.
     */
    void add(const StatementType &type);

    /**
     * Resolves an error model. Throws NameResolutionFailure if no error model
     * by the given name exists, OverloadResolutionFailure if no overload
     * exists for the given arguments, or otherwise returns the resolved error
     * model node.
     */
    semantic::ErrorModel resolve(const std::string &name, const Values &args) const;

};

/**
 * Extension of StatementType for instructions (aka gates) specifically, with
 * a few extra parameters.
 */
class InstructionType : public StatementType {

    /**
     * Whether this instruction supports conditional execution by means of the
     * c- notation. This is normally true.
     */
    bool allow_conditional;

    /**
     * Whether this instruction can be used in a bundle. This is normally true.
     */
    bool allow_parallel;

    /**
     * Whether to allow usage of the same qubit in different arguments. This is
     * normally false, as this makes no sense in QM, in which case libqasm will
     * report an error to the user if a qubit is reused. Setting this to true
     * just disables that check.
     */
    bool allow_reused_qubits;

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
    InstructionType(
        const std::string &name,
        const std::string &param_types = "",
        bool allow_conditional = true,
        bool allow_parallel = true,
        bool allow_reused_qubits = false);

};

/**
 * Table of the supported instructions and their overloads.
 */
class InstructionTable {
private:
    std::unique_ptr<OverloadedNameResolver<InstructionType>> resolver;
public:

    InstructionTable();

    /**
     * Registers an instruction type.
     */
    void add(const InstructionType &type);

    /**
     * Resolves an instruction. Throws NameResolutionFailure if no instruction
     * by the given name exists, OverloadResolutionFailure if no overload
     * exists for the given arguments, or otherwise returns the resolved
     * instruction node.
     */
    semantic::Instruction resolve(const std::string &name, const Values &args) const;

};

} // namespace types
} // namespace cqasm

#endif
