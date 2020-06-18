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

/**
 * Constructs a set of types from a shorthand string representation. In it,
 * each character represents one type. The supported characters are as follows:
 *
 *  - q = qubit
 *  - a = axis (x, y, or z)
 *  - b = bit/boolean
 *  - i = integer
 *  - r = real
 *  - c = complex
 *  - u = complex matrix of size 4^n, where n is the number of qubits in
 *        the parameter list (automatically deduced)
 *  - s = (quoted) string
 *  - j = json
 *
 * Note that complex matrices with different constraints and real matrices of
 * any kind cannot be specified this way. You'll have to construct and add
 * those manually.
 */
Types from_spec(const std::string &spec);

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
