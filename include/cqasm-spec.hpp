#ifndef _CQASM_SPEC_HPP_INCLUDED_
#define _CQASM_SPEC_HPP_INCLUDED_

namespace cqasm {
namespace spec {

namespace types {

class Type {
    virtual ~Type() {}
    virtual std::string name() const = 0;
};

class QubitType : public Type {
    std::string name() const override { "qubit" };
};

class IntType : public Type {
    std::string name() const override { "int" };
};

class RealType : public Type {
    std::string name() const override { "real" };
};

class ComplexType : public Type {
    std::string name() const override { "complex" };
};

class IntType : public Type {
    std::string name() const override { "int" };
};



}

} // namespace spec
} // namespace cqasm

#endif
