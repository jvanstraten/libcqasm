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
    virtual bool is_complete() {
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
    template <class V>
    void visit(V &visitor) {
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
class Any : public Node {
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
    template <class V>
    void visit(V &visitor) {
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
