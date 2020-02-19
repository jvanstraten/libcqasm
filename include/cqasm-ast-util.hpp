#ifndef _CQASM_AST_UTIL_HPP_INCLUDED_
#define _CQASM_AST_UTIL_HPP_INCLUDED_

#include <cstdint>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <vector>

namespace cqasm {
namespace ast {

class Visitor;

/**
 * String primitive used within the AST.
 */
using Str = std::string;

/**
 * Integer primitive used within the AST.
 */
using Int = std::int64_t;

/**
 * Real number primitive used within the AST.
 */
using Real = double;

/**
 * Base class for all AST components.
 */
class Base {
public:

    /**
     * We're using inheritance, so we need a virtual destructor for proper
     * cleanup.
     */
    virtual ~Base() {
    };

    /**
     * Returns whether this object is complete/fully defined.
     */
    virtual bool is_complete() {
        return true;
    }

    /**
     * Visit this object.
     */
    virtual void visit(Visitor &visitor) = 0;

    /**
     * Writes a debug dump of this object to the given stream.
     */
    void dump(std::ostream &out=std::cout);

};

/**
 * Convenience class for a reference to an optional AST node.
 */
template <class T>
class Maybe : public Base {
public:

    /**
     * The contained value. Must be non-null for a completed AST.
     */
    std::shared_ptr<T> val;

    /**
     * Sets the value to the given new-initialized class. Takes ownership of
     * the pointer.
     */
    void set_raw(T *ptr) {
        val = std::shared_ptr<T>(ptr);
    }

    /**
     * Returns whether this object is complete/fully defined.
     */
    bool is_complete() override {
        return !val || val->is_complete();
    }

    /**
     * Visit this object.
     */
    void visit(Visitor &visitor) override {
        if (val) {
            val->visit(visitor);
        }
    }

    /**
     * Returns whether this Maybe is empty.
     */
    virtual bool empty() {
        return !val;
    }

};

/**
 * Convenience class for a reference to exactly one other AST node.
 */
template <class T>
class One : public Maybe<T> {
public:

    /**
     * Returns whether this object is complete/fully defined.
     */
    bool is_complete() override {
        return this->val && this->val->is_complete();
    }

};

/**
 * Convenience class for zero or more AST nodes.
 */
template <class T>
class Any : public Base {
public:

    /**
     * The contained vector. The `shared_ptr`s may be assumed to be non-null.
     */
    std::vector<std::shared_ptr<T>> vec;

    /**
     * Appends the given new-initialized class. Takes ownership of the pointer.
     */
    void add_raw(T *ptr) {
        this->vec.emplace_back(ptr);
    }

    /**
     * Extends this Any with another.
     */
    void extend(Any<T> &other) {
        this->vec.insert(this->vec.end(), other.vec.begin(), other.vec.end());
    }

    /**
     * Returns whether this object is complete/fully defined.
     */
    bool is_complete() override {
        for (auto &sptr : this->vec) {
            if (!sptr->is_complete()) {
                return false;
            }
        }
        return true;
    }

    /**
     * Visit this object.
     */
    void visit(Visitor &visitor) override {
        for (auto &sptr : this->vec) {
            if (sptr) {
                sptr->visit(visitor);
            }
        }
    }

    /**
     * Returns whether this Any is empty.
     */
    virtual bool empty() {
        return vec.empty();
    }

};

/**
 * Convenience class for one or more AST nodes.
 */
template <class T>
class Many : public Any<T> {
public:

    /**
     * Returns whether this object is complete/fully defined.
     */
    bool is_complete() override {
        return !this->vec.empty() && Any<T>::is_complete();
    }

};

/**
 * Main class for AST nodes.
 */
class Node : public Base {
    // TODO: location information
};

} // namespace ast
} // namespace cqasm

/**
 * Stream << overload for AST nodes (writes debug dump).
 */
std::ostream& operator<<(std::ostream& os, const cqasm::ast::Base& object);

#endif
