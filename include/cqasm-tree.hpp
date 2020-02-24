#ifndef _CQASM_TREE_HPP_INCLUDED_
#define _CQASM_TREE_HPP_INCLUDED_

#include <memory>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <functional>

namespace cqasm {
namespace tree {

/**
 * Utility class for carrying any kind of value. Basically, `std::any` within
 * C++11.
 */
class Anything {
private:

    /**
     * Pointer to the contained data, or nullptr if no data is contained.
     */
    void *data;

    /**
     * Function used to free the contained data.
     */
    std::function<void(void *data)> destructor;

    /**
     * Type information.
     */
    std::type_index type;

    /**
     * Constructs an Anything object.
     */
    Anything(void *data, std::function<void(void *data)> destructor, std::type_index type) :
        data(data),
        destructor(destructor),
        type(type)
    {}

public:

    /**
     * Constructs an empty Anything object.
     */
    Anything() :
        data(nullptr),
        destructor([](void *data){}),
        type(std::type_index(typeid(nullptr)))
    {}

    /**
     * Constructs an Anything object by copying a value into it.
     */
    template <typename T>
    static Anything make(const T &ob) {
        return Anything(
            new T(ob),
            [](void *data) {
                delete static_cast<T*>(data);
            },
            std::type_index(typeid(T))
        );
    }

    /**
     * Constructs an Anything object by moving a value into it.
     */
    template <typename T>
    static Anything make(T &&ob) {
        return Anything(
            new T(std::move(ob)),
            [](void *data) {
                delete static_cast<T*>(data);
            },
            std::type_index(typeid(T))
        );
    }

    /**
     * Destructor.
     */
    ~Anything() {
        if (data) {
            destructor(data);
        }
    }

    // Anything objects are not copyable, because type information is lost
    // after the initial construction.
    Anything(const Anything&) = delete;
    Anything& operator=(const Anything&) = delete;

    /**
     * Move constructor.
     */
    Anything(Anything &&src) :
        data(src.data),
        destructor(std::move(src.destructor)),
        type(std::move(src.type))
    {
        src.data = nullptr;
    }

    /**
     * Move assignment.
     */
    Anything& operator=(Anything &&src) {
        if (data) {
            destructor(data);
        }
        data = src.data;
        destructor = std::move(src.destructor);
        type = std::move(src.type);
        src.data = nullptr;
        return *this;
    }

    /**
     * Returns a mutable pointer to the contents.
     *
     * @throws std::bad_cast when the type is incorrect.
     */
    template <typename T>
    T *get_mut() {
        if (std::type_index(typeid(T)) != type) {
            throw std::bad_cast();
        }
        return static_cast<T*>(data);
    }

    /**
     * Returns a const pointer to the contents.
     *
     * @throws std::bad_cast when the type is incorrect.
     */
    template <typename T>
    const T *get_const() const {
        if (std::type_index(typeid(T)) != type) {
            throw std::bad_cast();
        }
        return static_cast<T*>(data);
    }

};

/**
 * Base class for all tree nodes.
 */
class Node {
private:

    /**
     * The annotations stored with this node.
     */
    std::unordered_map<std::type_index, std::shared_ptr<Anything>> annotations;

public:

    /**
     * We're using inheritance, so we need a virtual destructor for proper
     * cleanup.
     */
    virtual ~Node() {
    };

    /**
     * Returns whether this object is complete/fully defined.
     */
    virtual bool is_complete() const {
        return true;
    }

    /**
     * Adds an annotation object to this node.
     *
     * Annotations are keyed by their type. That is, a node can contain zero or
     * one annotation for each C++ type.
     *
     * The annotations object is copied into the node. If you don't want to
     * make a copy, you can store a (smart) pointer to the object instead, in
     * which case the copied object is the pointer.
     */
    template <typename T>
    void set_annotation(const T &ob) {
        annotations[std::type_index(typeid(T))] = std::make_shared<Anything>(Anything::make<T>(ob));
    }

    /**
     * Adds an annotation object to this node.
     *
     * Annotations are keyed by their type. That is, a node can contain zero or
     * one annotation for each C++ type.
     *
     * The annotations object is moved into the node.
     */
    template <typename T>
    void set_annotation(T &&ob) {
        annotations[std::type_index(typeid(T))] = std::make_shared<Anything>(Anything::make<T>(std::move(ob)));
    }

    /**
     * Returns whether this object holds an annotation object of the given
     * type.
     */
    template <typename T>
    bool has_annotation() const {
        return annotations.count(std::type_index(typeid(T)));
    }

    /**
     * Returns a mutable pointer to the annotation object of the given type
     * held by this object, or `nullptr` if there is no such annotation.
     */
    template <typename T>
    T *get_annotation() {
        try {
            return annotations.at(std::type_index(typeid(T)))->get_mut<T>();
        } catch (const std::out_of_range&) {
            return nullptr;
        }
    }

    /**
     * Returns an immutable pointer to the annotation object of the given type
     * held by this object, or `nullptr` if there is no such annotation.
     */
    template <typename T>
    const T *get_annotation() const {
        try {
            return annotations.at(std::type_index(typeid(T)))->get_const<T>();
        } catch (const std::out_of_range&) {
            return nullptr;
        }
    }

};

/**
 * Convenience class for a reference to an optional AST node.
 */
template <class T>
class Maybe : public Node {
protected:

    /**
     * The contained value. Must be non-null for a completed AST.
     */
    std::shared_ptr<T> val;

public:

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
class Any : public Node {
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

/**
 * Code snippit for the `using` statements to make use of these tree base types
 * in a namespace for a certain kind of tree.
 */
#define USING_CQASM_TREE                                        \
    using Base = ::cqasm::tree::Node;                           \
    template <class T> using Maybe = ::cqasm::tree::Maybe<T>;   \
    template <class T> using One   = ::cqasm::tree::One<T>;     \
    template <class T> using Any   = ::cqasm::tree::Any<T>;     \
    template <class T> using Many  = ::cqasm::tree::Many<T>

} // namespace tree
} // namespace cqasm

#endif
