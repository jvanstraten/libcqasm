#include "cqasm-resolver.hpp"

namespace cqasm {
namespace resolver {

using Type = types::Type;
using Types = types::Types;
using Value = values::Value;
using Values = values::Values;

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

ErrorModelTable::ErrorModelTable() : resolver(new OverloadedNameResolver<error_model::ErrorModel>()) {}

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
 * model node.
 */
error_model::ErrorModel ErrorModelTable::resolve(const std::string &name, const Values &args) const {
    // TODO
}

InstructionTable::InstructionTable() : resolver(new OverloadedNameResolver<instruction::Instruction>()) {}

/**
 * Registers an instruction type.
 */
void InstructionTable::add(const instruction::Instruction &type) {
    resolver->add_overload(type.name, type, type.param_types);
}

/**
 * Resolves an instruction. Throws NameResolutionFailure if no instruction
 * by the given name exists, OverloadResolutionFailure if no overload
 * exists for the given arguments, or otherwise returns the resolved
 * instruction node.
 */
instruction::Instruction InstructionTable::resolve(const std::string &name, const Values &args) const {
    // TODO
}

} // namespace resolver
} // namespace cqasm
