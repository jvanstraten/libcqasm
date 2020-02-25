#include "cqasm-primitives.hpp"

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
