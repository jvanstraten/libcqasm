#pragma once

#include "cqasm-types-gen.hpp"

namespace cqasm {
namespace types {

/**
 * A cQASM type.
 */
using Type = tree::One<Node>;

/**
 * Zero or more cQASM types.
 */
using Types = tree::Any<Node>;

} // namespace types
} // namespace cqasm

/**
 * Stream << overload for a single type.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::types::Type& type);

/**
 * Stream << overload for zero or more types.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::types::Types& types);
