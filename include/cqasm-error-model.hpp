#pragma once

#include "cqasm-types.hpp"
#include "cqasm-values.hpp"

namespace cqasm {
namespace error_model {

/**
 * Representation of an error model. A number of these can be registered into
 * libqasm by the program or library using it, to inform libqasm about which
 * error models are supported. libqasm needs to know its name and which
 * parameters it expects in order to be able to resolve the error model
 * information specified in the cQASM file. The resolved error model (if any
 * is specified in the file) can then be read from the parse result.
 *
 * Note that it is legal to have multiple error models with the same name, as
 * long as they can be distinguished through their parameter types (i.e. the
 * available error models can be overloaded).
 *
 * You can add any data you like to these through the Annotatable interface
 * for your own bookkeeping, so you don't have to maintain an additional map
 * from this error model structure to your own internal structure if you're
 * okay with using this one.
 */
class ErrorModel : public annotatable::Annotatable {
public:

    /**
     * The name of the error model. Names are matched case insensitively.
     */
    std::string name;

    /**
     * The vector of parameter types that this error model expects.
     */
    types::Types param_types;

    /**
     * Creates a new error model. param_types is a shorthand type specification
     * string as parsed by cqasm::types::from_spec(). If you need more control,
     * you can also manipulate param_types directly.
     */
    ErrorModel(const std::string &name, const std::string &param_types = "");

    /**
     * Equality operator.
     */
    bool operator==(const ErrorModel& rhs) const;

    /**
     * Inequality operator.
     */
    inline bool operator!=(const ErrorModel& rhs) const {
        return !(*this == rhs);
    }

};

} // namespace error_model
} // namespace cqasm

/**
 * Stream << overload for error models.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::error_model::ErrorModel& model);
