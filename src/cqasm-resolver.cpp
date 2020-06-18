#include <unordered_set>
#include "cqasm-resolver.hpp"

namespace cqasm {
namespace resolver {

using Type = types::Type;
using Types = types::Types;
using Value = values::Value;
using Values = values::Values;

/**
 * Makes a string lowercase.
 */
std::string lowercase(const std::string &name) {
    std::string name_lower = name;
    std::for_each(name_lower.begin(), name_lower.end(), [](char &c){
        c = std::tolower(c);
    });
    return name_lower;
}

/**
 * Adds a mapping.
 */
void MappingTable::add(const std::string &name, const Value &value) {
    table.insert(std::make_pair(lowercase(name), value));
}

/**
 * Resolves a mapping. Throws NameResolutionFailure if no mapping by the
 * given name exists.
 */
Value MappingTable::resolve(const std::string &name) const {
    auto entry = table.find(lowercase(name));
    if (entry == table.end()) {
        throw NameResolutionFailure();
    } else {
        return Value(entry->second->clone());
    }
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
                promoted_args.add(promoted_arg);
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
        std::string name_lower = lowercase(name);
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
        std::string name_lower = lowercase(name);
        auto entry = table.find(name_lower);
        if (entry == table.end()) {
            throw NameResolutionFailure();
        } else {
            return entry->second.resolve(args);
        }
    }

};

FunctionTable::FunctionTable() : resolver(new OverloadedNameResolver<FunctionImpl>()) {}
FunctionTable::~FunctionTable() {}
FunctionTable::FunctionTable(const FunctionTable& t) : resolver(new OverloadedNameResolver<FunctionImpl>(*t.resolver)) {}
FunctionTable::FunctionTable(FunctionTable&& t) : resolver(std::move(t.resolver)) {}
FunctionTable& FunctionTable::operator=(const FunctionTable& t) {
    resolver = std::unique_ptr<OverloadedNameResolver<FunctionImpl>>(new OverloadedNameResolver<FunctionImpl>(*t.resolver));
}
FunctionTable& FunctionTable::operator=(FunctionTable&& t) {
    resolver = std::move(t.resolver);
}

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

ErrorModelTable::ErrorModelTable() : resolver(new OverloadedNameResolver<error_model::ErrorModel>()) {}
ErrorModelTable::~ErrorModelTable() {}
ErrorModelTable::ErrorModelTable(const ErrorModelTable& t) : resolver(new OverloadedNameResolver<error_model::ErrorModel>(*t.resolver)) {}
ErrorModelTable::ErrorModelTable(ErrorModelTable&& t) : resolver(std::move(t.resolver)) {}
ErrorModelTable& ErrorModelTable::operator=(const ErrorModelTable& t) {
    resolver = std::unique_ptr<OverloadedNameResolver<error_model::ErrorModel>>(new OverloadedNameResolver<error_model::ErrorModel>(*t.resolver));
}
ErrorModelTable& ErrorModelTable::operator=(ErrorModelTable&& t) {
    resolver = std::move(t.resolver);
}

/**
 * Registers an error model.
 */
void ErrorModelTable::add(const error_model::ErrorModel &type) {
    resolver->add_overload(type.name, type, type.param_types);
}

/**
 * Resolves an error model. Throws NameResolutionFailure if no error model
 * by the given name exists, OverloadResolutionFailure if no overload
 * exists for the given arguments, or otherwise returns the resolved error
 * model node. Annotation data and line number information still needs to be
 * set by the caller.
 */
semantic::ErrorModel ErrorModelTable::resolve(const std::string &name, const Values &args) const {
    auto resolved = resolver->resolve(name, args);
    return semantic::ErrorModel(
        resolved.first,
        name,
        resolved.second,
        tree::Any<semantic::AnnotationData>());
}

InstructionTable::InstructionTable() : resolver(new OverloadedNameResolver<instruction::Instruction>()) {}
InstructionTable::~InstructionTable() {}
InstructionTable::InstructionTable(const InstructionTable& t) : resolver(new OverloadedNameResolver<instruction::Instruction>(*t.resolver)) {}
InstructionTable::InstructionTable(InstructionTable&& t) : resolver(std::move(t.resolver)) {}
InstructionTable& InstructionTable::operator=(const InstructionTable& t) {
    resolver = std::unique_ptr<OverloadedNameResolver<instruction::Instruction>>(new OverloadedNameResolver<instruction::Instruction>(*t.resolver));
}
InstructionTable& InstructionTable::operator=(InstructionTable&& t) {
    resolver = std::move(t.resolver);
}

/**
 * Registers an instruction type.
 */
void InstructionTable::add(const instruction::Instruction &type) {
    resolver->add_overload(type.name, type, type.param_types);
}

/**
 * Resolves an instruction. This can result in any of the following things:
 *
 *  - There is no registered instruction by the given name. This throws a
 *    NameResolutionFailure.
 *  - The name is known, but there is no overload for the given parameter list.
 *    This throws an OverloadResolutionFailure.
 *  - Conditional execution (c-) notation was used, but the instruction doesn't
 *    support it. This throws a ConditionalExecutionNotSupported.
 *  - One or more qubits are used more than once in the instruction, and the
 *    instruction doesn't support this. This throws a QubitsNotUnique.
 *  - Conditional execution (c-) notation was used and is supported, and the
 *    condition resolves to constant false. In this case, an empty Maybe is
 *    returned.
 *  - The common case: a filled Maybe is returned with the resolved instruction
 *    node. Annotation data and line number information still needs to be
 *    copied from the AST by the caller.
 */
tree::Maybe<semantic::Instruction> InstructionTable::resolve(
    const std::string &name,
    const Value &condition,
    const Values &args
) const {

    // Resolve the instruction name and overload.
    auto resolved = resolver->resolve(name, args);
    auto &insn = resolved.first;
    auto &res_args = resolved.second;

    // Enforce qubit uniqueness if the instruction requires us to.
    if (!insn.allow_reused_qubits) {
        std::unordered_set<primitives::Int> qubits_used;
        for (const auto &arg : res_args) {
            if (auto x = arg->as_qubit_refs()) {
                for (auto index : x->index) {
                    if (!qubits_used.insert(index).second) {
                        throw QubitsNotUnique();
                    }
                }
            }
        }
    }

    // Resolve the condition code.
    Value res_condition;
    if (condition) {
        if (!insn.allow_conditional) {
            throw ConditionalExecutionNotSupported();
        }
        res_condition = values::promote(condition, tree::make<types::Bool>());
        if (auto x = res_condition->as_const_bool()) {
            if (!x->value) {
                return tree::Maybe<semantic::Instruction>();
            }
        }
    } else {
        res_condition.set(tree::make<values::ConstBool>(true));
    }

    // Construct the bound instruction node.
    return tree::make<semantic::Instruction>(
        insn, name, res_condition, res_args,
        tree::Any<semantic::AnnotationData>());
}

} // namespace resolver
} // namespace cqasm
