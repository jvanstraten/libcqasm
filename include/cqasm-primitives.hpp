#ifndef _CQASM_PRIMITIVES_HPP_INCLUDED_
#define _CQASM_PRIMITIVES_HPP_INCLUDED_

#include <string>
#include <cstdint>
#include <complex>
#include <vector>

namespace cqasm {
namespace primitives {

/**
 * String primitive used within the AST and semantic trees.
 */
using Str = std::string;

/**
 * Boolean primitive used within the semantic trees.
 */
using Bool = bool;

/**
 * Integer primitive used within the AST and semantic trees.
 */
using Int = std::int64_t;

/**
 * Real number primitive used within the AST and semantic trees.
 */
using Real = double;

/**
 * Complex number primitive used within the semantic trees.
 */
using Complex = std::complex<double>;

/**
 * Two-dimensional matrix of some kind of type.
 */
template <typename T>
class Matrix {

    // TODO

};

/**
 * Matrix of real numbers.
 */
using RMatrix = Matrix<Real>;

/**
 * Matrix of complex numbers.
 */
using CMatrix = Matrix<Complex>;

/**
 * Version number primitive used within the AST and semantic trees.
 */
class Version : public std::vector<Int> {
public:
    // TODO: relational operators
};

} // namespace primitives
} // namespace cqasm

#endif
