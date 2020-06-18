#include "cqasm-types.hpp"

namespace cqasm {
namespace types {

} // namespace overload
} // namespace types

/**
 * Stream << overload for a single type.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::types::Type& type) {
    if (type.empty()) {
        os << "NULL";
    } else {
        os << type.get();
    }
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
}
