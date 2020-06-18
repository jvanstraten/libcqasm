#pragma once

#include "cqasm-ast.hpp"
#include "cqasm-values-gen.hpp"
#include "cqasm-types.hpp"

namespace cqasm {
namespace values {

/**
 * A cQASM value, either known at compile-time or an expression for something
 * only known at runtime.
 */
using Value = tree::One<Node>;

/**
 * Zero or more cQASM values.
 */
using Values = tree::Any<Node>;

/**
 * Type-checks and (if necessary) promotes the given value to the given type.
 * Returns null if the check/promotion fails, otherwise returns the constructed
 * value by way of a smart pointer.
 */
Value promote(const Value &value, const types::Type &type);

} // namespace values
} // namespace cqasm

/**
 * Stream << overload for a single value.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::values::Value& value);

/**
 * Stream << overload for zero or more values.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::values::Values& values);
