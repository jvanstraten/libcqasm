#include "cqasm-types.hpp"

namespace cqasm {
namespace types {

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
Types from_spec(const std::string &spec) {
    // Count the number of qubits, in case we find a unitary parameter.
    size_t num_qubits = 0;
    for (auto c : spec) {
        if (c == 'q') {
            num_qubits += 1;
        }
    }

    // Now resolve the types.
    Types types;
    for (auto c : spec) {
        switch (c) {
            case 'q':
                types.add_raw(new types::Qubit());
                break;
            case 'a':
                types.add_raw(new types::Axis());
                break;
            case 'b':
                types.add_raw(new types::Bool());
                break;
            case 'i':
                types.add_raw(new types::Int());
                break;
            case 'r':
                types.add_raw(new types::Real());
                break;
            case 'c':
                types.add_raw(new types::Complex());
                break;
            case 'u':
                types.add_raw(new types::ComplexMatrix(
                    1ull << num_qubits, 1ull << num_qubits));
                break;
            case 's':
                types.add_raw(new types::String());
                break;
            case 'j':
                types.add_raw(new types::Json());
                break;
            default:
                throw std::invalid_argument("unknown type code encountered");
        }
    }
    return types;
}

} // namespace types
} // namespace cqasm

/**
 * Stream << overload for a single type.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::types::Type& type) {
    if (type.empty()) {
        os << "NULL";
    } else {
        os << *type;
    }
    return os;
}

/**
 * Stream << overload for zero or more types.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::types::Types& types) {
    os << "[";
    bool first = true;
    for (const auto &type : types) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        if (type) {
            os << *type;
        } else {
            os << "NULL";
        }
    }
    os << "]";
    return os;
}
