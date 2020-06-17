#include "cqasm-primitives.hpp"

/**
 * Stream << overload for axis nodes.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::primitives::Axis& axis) {
    switch (axis) {
        case ::cqasm::primitives::Axis::X:
            os << "X";
            break;
        case ::cqasm::primitives::Axis::Y:
            os << "Y";
            break;
        case ::cqasm::primitives::Axis::Z:
            os << "Z";
            break;
    }
    return os;
}

/**
 * Stream << overload for version nodes.
 */
std::ostream& operator<<(std::ostream& os, const ::cqasm::primitives::Version& object) {
    bool first = true;
    for (auto item : object) {
        if (first) {
            first = false;
        } else {
            os << ".";
        }
        os << item;
    }
    return os;
}
