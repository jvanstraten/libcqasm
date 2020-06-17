/** \file
 * Defines base types for constructing structured trees, such as the AST.
 *
 * The different kinds of nodes in a structured tree are represented by
 * different class types in C++. Inheritance is used for
 * specialized/generalized types, for example for the relation between an
 * expression and a binary operator; a binary operator is a speciailization of
 * an expression, so it derives from the expression node class. All node types
 * should ultimately inherit from `Base`.
 *
 * The children of the nodes are represented as class members. Instead of
 * containing the child node directly, which would cause problems with
 * recursive tree definitions, they should be encapsulated in one of:
 *
 *  - `Maybe<ChildNode>`: zero or one child node;
 *  - `One<ChildNode>`: exactly one child node;
 *  - `Any<ChildNode>`: zero or more child nodes; or
 *  - `Many<ChildNode>`: one or more child nodes.
 *
 * The `is_complete()` function can be used to recursively check whether the
 * above constraints are met: `One` and `Many` can in fact be empty. This makes
 * progressively constructing the tree easier.
 *
 * Besides the child nodes, nodes can also be given annotations. Annotations
 * can be any kind of object; in fact they are identified by their type, so
 * each node can have zero or one instance of every C++ type associated with
 * it. They allow users of the tree to attach their own data to the recursive
 * structure, without the designer of the tree having to know about it in
 * advance. This hopefully makes the tree structure maintainable, while still
 * being usable by the users of the library, without them having to convert to
 * their own representation first.
 *
 * To do the above for implementations (member functions) as well, the visitor
 * pattern is recommended. Refer to the `cqasm-ast.hpp` header for details.
 */

#ifndef _CQASM_TREE_HPP_INCLUDED_
#define _CQASM_TREE_HPP_INCLUDED_

#include <memory>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <functional>
#include "cqasm-annotatable.hpp"

namespace cqasm {
namespace tree {

/**
 * Base class for all tree nodes.
 */
class Base : public annotatable::Annotatable {
public:

    /**
     * Returns whether this object is complete/fully defined.
     */
    virtual bool is_complete() const {
        return true;
    }

};

/**
 * Convenience class for a reference to an optional AST node.
 */
template <class T>
class Maybe : public Base {
protected:

    /**
     * The contained value. Must be non-null for a completed AST.
     */
    std::shared_ptr<T> val;

public:

    /**
     * Constructor for an empty node.
     */
    Maybe() : val() {}

    /**
     * Constructor for a filled node (copy).
     */
    explicit Maybe(const T &value) : val(new T(value)) {}

    /**
     * Constructor for a filled node (move).
     */
    explicit Maybe(T &&value) : val(new T(std::move(value))) {}

    /**
     * Constructor for a filled node. The given raw pointer must be
     * new-allocated.
     */
    explicit Maybe(T *value) : val(value) {}

    /**
     * Sets the value to a copy of the object pointed to, or clears it if null.
     */
    void set(const T *ob) {
        if (ob) {
            val = std::make_shared(*ob);
        } else {
            val.reset();
        }
    }

    /**
     * Sets the value to a copy of the object pointed to.
     */
    Maybe &operator=(const T *ob) {
        set(*ob);
    }

    /**
     * Sets the value to a copy of the given object.
     */
    void set(const T &ob) {
        val = std::make_shared(ob);
    }

    /**
     * Sets the value to a copy of the given object.
     */
    Maybe &operator=(const T &ob) {
        set(ob);
    }

    /**
     * Move the given object into the contained value.
     */
    void set(T &&ob) {
        val = std::make_shared(std::move(ob));
    }

    /**
     * Sets the value to a copy of the given object.
     */
    Maybe &operator=(T &&ob) {
        set(std::move(ob));
    }

    /**
     * Set the given object by means of a `shared_ptr`.
     */
    void set(std::shared_ptr<T> &ob) {
        val = ob;
    }

    /**
     * Sets the value to a copy of the given object.
     */
    Maybe &operator=(std::shared_ptr<T> &ob) {
        set(ob);
    }

    /**
     * Sets the value to the given new-initialized class. Takes ownership of
     * the pointer.
     *
     * @warning Do NOT give a pointer to a statically allocated,
     * stack-allocated, or `malloc`-allocated object to this function, because
     * then the destructor will be wrong. You will then get weird runtime errors
     * when the object is destroyed. Use `set()` or `operator=` instead.
     */
    void set_raw(T *ptr) {
        val = std::shared_ptr<T>(ptr);
    }

    /**
     * Removes the contained value.
     */
    void reset() {
        val.reset();
    }

    /**
     * Returns whether this Maybe is empty.
     */
    virtual bool empty() const {
        return !val;
    }

    /**
     * Returns whether this Maybe is empty.
     */
    size_t size() const {
        return val ? 1 : 0;
    }

    /**
     * Returns a mutable reference to the contained value. Raises an
     * `out_of_range` when the reference is empty.
     */
    T &get() {
        if (!val) {
            throw std::out_of_range("dereferencing empty Maybe/One object");
        }
        return *val;
    }

    /**
     * Returns a const reference to the contained value. Raises an
     * `out_of_range` when the reference is empty.
     */
    const T &get() const {
        if (!val) {
            throw std::out_of_range("dereferencing empty Maybe/One object");
        }
        return *val;
    }

    /**
     * Mutable dereference operator, shorthand for `get()`.
     */
    T &operator*() {
        return get();
    }

    /**
     * Constant dereference operator, shorthand for `get()`.
     */
    const T &operator*() const {
        return get();
    }

    /**
     * Mutable dereference operator, shorthand for `get()`.
     */
    T *operator->() {
        return &get();
    }

    /**
     * Constant dereference operator, shorthand for `get()`.
     */
    const T *operator->() const {
        return &get();
    }

    /**
     * Equality operator.
     */
    bool operator==(const Maybe& rhs) const {
        return val == rhs.val;
    }

    /**
     * Inequality operator.
     */
    inline bool operator!=(const Maybe& rhs) const {
        return !(*this == rhs);
    }

    /**
     * Returns whether this object is complete/fully defined.
     */
    bool is_complete() const override {
        return !val || val->is_complete();
    }

    /**
     * Visit this object.
     */
    template <class V>
    void visit(V &visitor) {
        if (val) {
            val->visit(visitor);
        }
    }

};

/**
 * Convenience class for a reference to exactly one other AST node.
 */
template <class T>
class One : public Maybe<T> {
public:

    /**
     * Constructor for an empty (invalid) node.
     */
    One() : Maybe<T>() {}

    /**
     * Constructor for a filled node (copy).
     */
    explicit One(const T &value) : Maybe<T>(value) {}

    /**
     * Constructor for a filled node (move).
     */
    explicit One(T &&value) : Maybe<T>(std::move(value)) {}

    /**
     * Constructor for a filled node. The given raw pointer must be
     * new-allocated.
     */
    explicit One(T *value) : Maybe<T>(value) {}

    /**
     * Returns whether this object is complete/fully defined.
     */
    bool is_complete() const override {
        return this->val && this->val->is_complete();
    }

};

/**
 * Convenience class for zero or more AST nodes.
 */
template <class T>
class Any : public Base {
protected:

    /**
     * The contained vector. The `shared_ptr`s are assumed to be non-null.
     */
    std::vector<std::shared_ptr<T>> vec;

public:

    /**
     * Adds the value pointed to by copy. No-operation when ob is null.
     */
    void add(const T *ob, ssize_t pos=-1) {
        if (!ob) {
            return;
        }
        if (pos < 0 || pos >= size()) {
            this->vec.emplace_back(std::make_shared(*ob));
        } else {
            this->vec.emplace(this->vec.cbegin() + pos, std::make_shared(*ob));
        }
    }

    /**
     * Adds the given value by copy.
     */
    void add(const T &ob, ssize_t pos=-1) {
        if (pos < 0 || pos >= size()) {
            this->vec.emplace_back(std::make_shared(ob));
        } else {
            this->vec.emplace(this->vec.cbegin() + pos, std::make_shared(ob));
        }
    }

    /**
     * Adds the given value by move.
     */
    void add(T &&ob, ssize_t pos=-1) {
        if (pos < 0 || pos >= size()) {
            this->vec.emplace_back(std::make_shared(std::move(ob)));
        } else {
            this->vec.emplace(this->vec.cbegin() + pos, std::make_shared(std::move(ob)));
        }
    }

    /**
     * Set the given object by means of a `shared_ptr`. No operation when op is
     * null.
     */
    void add(std::shared_ptr<T> &ob, ssize_t pos=-1) {
        if (!ob) {
            return;
        }
        if (pos < 0 || pos >= size()) {
            this->vec.emplace_back(ob);
        } else {
            this->vec.emplace(this->vec.cbegin() + pos, ob);
        }
    }

    /**
     * Appends the given new-initialized class. Takes ownership of the pointer.
     *
     * @warning Do NOT give a pointer to a statically allocated,
     * stack-allocated, or `malloc`-allocated object to this function, because
     * then the destructor will be wrong. You will then get weird runtime errors
     * when the object is destroyed. Use `set()` or `operator=` instead.
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
     * Removes the object at the given index, or at the back if no index is
     * given.
     */
    void remove(ssize_t pos=-1) {
        if (size() == 0) {
            return;
        }
        if (pos < 0 || pos >= size()) {
            pos = size() - 1;
        }
        this->vec.erase(this->vec.cbegin() + pos);
    }

    /**
     * Removes the contained values.
     */
    void reset() {
        vec.clear();
    }

    /**
     * Returns whether this Any is empty.
     */
    virtual bool empty() const {
        return vec.empty();
    }

    /**
     * Returns the number of elements in this Any.
     */
    size_t size() const {
        return vec.size();
    }

    /**
     * Returns a mutable reference to the contained value at the given index.
     * Raises an `out_of_range` when the reference is empty.
     */
    T &get(size_t index) {
        return vec.at(index);
    }

    /**
     * Returns a mutable reference to the contained value at the given index.
     * Raises an `out_of_range` when the reference is empty.
     */
    const T &get(size_t index) const {
        return vec.at(index);
    }

    /**
     * Shorthand for `get()`.
     */
    T &operator[] (size_t index) {
        return get(index);
    }

    /**
     * Shorthand for `get()`.
     */
    const T &operator[] (size_t index) const {
        return get(index);
    }

    /**
     * `begin()` for for-each loops.
     */
    typename std::vector<std::shared_ptr<T>>::iterator begin() {
        return vec.begin();
    }

    /**
     * `begin()` for for-each loops.
     */
    typename std::vector<std::shared_ptr<T>>::const_iterator begin() const {
        return vec.begin();
    }

    /**
     * `end()` for for-each loops.
     */
    typename std::vector<std::shared_ptr<T>>::iterator end() {
        return vec.end();
    }

    /**
     * `end()` for for-each loops.
     */
    typename std::vector<std::shared_ptr<T>>::const_iterator end() const {
        return vec.end();
    }

    /**
     * Equality operator.
     */
    bool operator==(const Any& rhs) const {
        return vec == rhs.vec;
    }

    /**
     * Inequality operator.
     */
    inline bool operator!=(const Any& rhs) const {
        return !(*this == rhs);
    }

    /**
     * Returns whether this object is complete/fully defined.
     */
    bool is_complete() const override {
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
    template <class V>
    void visit(V &visitor) {
        for (auto &sptr : this->vec) {
            if (sptr) {
                sptr->visit(visitor);
            }
        }
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
    bool is_complete() const override {
        return !this->vec.empty() && Any<T>::is_complete();
    }

};

} // namespace tree
} // namespace cqasm

#endif
