#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include <cstdio>
#include <cassert>

/** @file sequence.cc */

/**
 * @class OwningPtr
 * std::unique_ptr wrapper allowing for copying owned value
 */
template <typename T>
struct OwningPtr {
    using Inner   = std::unique_ptr<T>;
    using Pointer = typename Inner::pointer;

    Inner inner;

    constexpr OwningPtr() = default;
    constexpr OwningPtr(std::nullptr_t) : inner {} {}

    explicit OwningPtr(Pointer p) : inner { p } {}
    
    OwningPtr(OwningPtr&&) = default;
    OwningPtr(Inner&& other) : inner { std::move(other) } {}

    auto operator =(OwningPtr&& rhs) -> OwningPtr& {
        inner = std::move(rhs.inner);
        return *this;
    }

    OwningPtr(OwningPtr const& other)
        : inner { other.inner ? new T { *other.inner } : nullptr } {}
    OwningPtr(Inner const& other)
        : inner { other ? new T { *other } : nullptr } {}

    auto operator =(OwningPtr const& rhs) -> OwningPtr& {
        inner.reset(rhs.inner ? new T { *rhs.inner } : nullptr);
        return *this;
    }
};

/**
 * @fn make_owning
 * helper function for creating OwningPtr instances,
 * analogous to std::make_uniuqe
 * @return instance of OwningPtr<T> created with args
 */
template <typename T, typename... Args>
auto make_owning(Args&&... args) -> OwningPtr<T> {
    return OwningPtr<T> { new T { std::forward<Args>(args)... } };
}


template <typename Key, typename Info>
struct Sequence {
    /**
     * @class Node
     * used as a node of a Sequence
     */
    struct Node {
        using Elem = std::pair<Key, Info>;

        template <typename Key_, typename Info_>
        Node(Key_&& k, Info_&& i)
            : elem_ { std::forward<Key_>(k), std::forward<Info_>(i) }
            , next_ {}
        {}

        Node(Elem const& elem)
            : elem_ { elem }
            , next_ {}
        {}

        Node(Elem&& elem)
            : elem_ { std::move(elem) }
            , next_ {}
        {}

        template <typename Key_, typename Info_, typename... Ts>
        Node(Key_&& k, Info_&& i, Ts&&... vs)
            : elem_ { std::forward<Key_>(k), std::forward<Info_>(i) }
            , next_ { make_owning<Node>(std::forward<Ts>(vs)...) }
        {}

        template <typename... Ts>
        Node(Elem const& elem, Ts&&... vs)
            : elem_ { elem }
            , next_ { make_owning<Node>(std::forward<Ts>(vs)...) }
        {}

        template <typename... Ts>
        Node(Elem&& elem, Ts&&... vs)
            : elem_ { std::move(elem) }
            , next_ { make_owning<Node>(std::forward<Ts>(vs)...) }
        {}

        Node(Node const&) = default;
        Node(Node&&)      = default;

        auto print() const -> void {
            std::cout << elem_.first << ", " << elem_.second;
        }

        auto elem() const -> Elem const& { return elem_; }
        auto elem()       -> Elem&       { return elem_; }

        auto next() const -> OwningPtr<Node> const& { return next_; }
        auto next()       -> OwningPtr<Node>&       { return next_; }

    private:
        Elem            elem_;
        OwningPtr<Node> next_ = nullptr;
    };

    Sequence() = default;

    template <typename T, typename... Ts,
              typename = std::enable_if_t<
                  sizeof...(Ts) != 0 || !std::is_same_v<
                      std::remove_reference_t<T>,
                      Sequence
                      >
                  >
              >
    Sequence(T&& t, Ts&&... vs)
        : head_ { make_owning<Node>(std::forward<T>(t),
                                    std::forward<Ts>(vs)...) }
    {}

    Sequence(Sequence const& other) : head_ { other.head_ } {}

    Sequence(Sequence&& other) : head_ { std::move(other.head_) } {}

    auto empty() const -> bool { return !head_.inner; }

    auto clear() -> void {
        head_.inner.reset();
    }

    auto print() const -> void {
        if (empty()) { return; }
        std::cout << "head ";
        for (auto const& [key, info] : *this) {
            std::cout << " -> " << key << ", " << info;
        }

        std::cout << '\n';
    }

    /**
     * returns reference to first node, fires assertion  if sequence is empty.
     * @return reference to first node in sequence
     */
    auto first() const -> Node const& {
        assert(!empty() && "using first on empty list");
        return *head_.inner;
    }

    /**
     * returns reference to first node, fires assertion  if sequence is empty.
     * @return reference to first node in sequence
     */
    auto first() -> Node& {
        return const_cast<Node&>(const_cast<Sequence const*>(this)->first());
    }

    /**
     * returns reference to last node, fires assertion if sequence is empty.
     * @return reference to last node in sequence
     */
    auto last() const -> Node const& {
        assert(!empty() && "using last on empty list");
        auto node = std::cref(head_);
        while (node.get().inner->next().inner) {
            node = node.get().inner->next();
        }
        return *node.get().inner;
    }

    /**
     * returns reference to last node, fires assertion if sequence is empty.
     * @return reference to last node in sequence
     */
    auto last() -> Node& {
        return const_cast<Node&>(const_cast<Sequence const*>(this)->last());
    }

    /**
     * inserts a sequence or a single node in front of the sequence.
     * on empty sequence equivalent to append
     * @see append
     * @return self
     */
    template <typename... Ts>
    auto insert(Ts&&... vs) -> Sequence& {
        auto seq = Sequence { std::forward<Ts>(vs)... };
        if (empty()) {
            head_ = seq.head_;
        } else {
            seq.last().next() = OwningPtr<Node> { head_.inner.release() };
            head_             = OwningPtr<Node> { seq.head_.inner.release() };
        }
        return *this;
    }

    /**
     * appends a sequence or a single node at the end of the sequence.
     * on empty sequence equivalent to insert
     * @see insert
     * @return self
     */
    template <typename... Ts>
    auto append(Ts&&... vs) -> Sequence& {
        auto seq = Sequence { std::forward<Ts>(vs)... };
        if (empty()) {
            head_ = seq.head_;
        } else {
            last().next() = seq.head_;
        }
        return *this;
    }

    /**
     * removes and returns first element of the sequence, asserts on empty.
     * @return removed element
     */
    auto popf() -> typename Node::Elem {
        assert(!empty() && "popf on empty sequence");
        auto ret = head_.inner->elem();
        head_.inner.reset(head_.inner->next().inner.release());
        return ret;
    }

    /**
     * removes and returns last element of the sequence, assserts on empty.
     * @return removed element
     */
    auto popb() -> typename Node::Elem {
        assert(!empty() && "popb on empty sequence");
        auto node = std::ref(head_);
        while (node.get().inner->next().inner) {
            node = node.get().inner->next();
        }
        auto ret = node.get().inner->elem();
        node.get().inner.reset();
        return ret;
    }

    /**
     * @class Iterator
     * type used to iterate non-const Sequence
     */
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type        = typename Node::Elem;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type*;
        using reference         = value_type&;
        
        std::reference_wrapper<OwningPtr<Node>> elem_;

        auto operator ++() -> Iterator& {
            elem_ = elem_.get().inner->next();
            return *this;
        }

        auto operator *() const -> reference {
            return elem_.get().inner->elem();
        }
    };

    auto begin() -> Iterator { return Iterator { head_ }; }

    struct IterEnd {};

    auto end() -> IterEnd { return IterEnd {}; }

    friend auto operator !=(Iterator const& iter, IterEnd const&) -> bool {
        return iter.elem_.get().inner != nullptr;
    }

    /**
     * @class ConstIterator
     * type used to iterate const Sequence
     */
    struct ConstIterator {
        using iterator_category = std::output_iterator_tag;
        using value_type        = typename Node::Elem;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type const*;
        using reference         = value_type const&;

        std::reference_wrapper<OwningPtr<Node> const> elem_;

        auto operator ++() -> ConstIterator& {
            elem_ = elem_.get().inner->next();
            return *this;
        }

        auto operator *() const -> reference {
            return elem_.get().inner->elem();
        }
    };

    auto begin() const -> ConstIterator { return ConstIterator { head_ }; }

    auto end() const -> IterEnd { return IterEnd {}; }

    friend auto operator !=(ConstIterator const& iter, IterEnd const&) -> bool {
        return iter.elem_.get().inner != nullptr;
    }

    /**
     * insert node or nodes at position denoted by iterator.
     * @return self
     */
    template <typename... Ts>
    auto insert_at(Iterator const& iter, Ts&&... vs) -> Sequence& {
        auto seq = Sequence { std::forward<Ts>(vs)... };
        seq.last().next() = OwningPtr<Node> { iter.elem_.get().inner.release() };
        iter.elem_.get() = OwningPtr<Node> { seq.head_.inner.release() };
        return *this;
    }

    /**
     * get iterator at elem for which func returned true.
     * if no such elem exists return iterator to one pass the end
     */
    template <typename F>
    auto get_iter_by(F const& func) -> Iterator {
        auto it = begin();
        for (; it != end(); ++it) {
            if (func(*it)) { break; }
        }
        return it;
    }

    /**
     * get iterator at elem for which func returned true.
     * if no such elem exists return iterator to one pass the end
     */
    template <typename F>
    auto get_iter_by(F const& func) const -> ConstIterator {
        auto it = begin();
        for (; it != end(); ++it) {
            if (func(*it)) { break; }
        }
        return it;
    }
    
    /**
     * returns elem for which func returned true.
     * if no such elem exists fires assertion
     * @return elem fulfilling predicate func
     */
    template <typename F>
    auto get_elem_by(F const& func) const -> typename Node::Elem const& {
        auto it = get_iter_by(func);
        assert(it != end() && "no such element");
        return *it;
    }

    /**
     * returns elem for which func returned true.
     * if no such elem exists fires assertion
     * @return elem fulfilling predicate func
     */
    template <typename F>
    auto get_elem_by(F const& func) -> typename Node::Elem& {
        return const_cast<typename Node::Elem&>(
            const_cast<Sequence const*>(this)->get_elem_by(func)
            );
    }

    /**
     * remove first elem fulfilling predicate func.
     * if no elem does do nothing
     * @return self
     */
    template <typename F>
    auto remove_if(F const& func) -> Sequence& {
        auto it = get_iter_by(func);
        if (it != end()) {
            it.elem_.get() = it.elem_.get().inner->next();
        }
        return *this;
    }

private:
    OwningPtr<Node> head_ = nullptr;
};

/**
 * @fn make_seq
 * helper funciton for creating Sequence instances.
 * @return instance of Sequence
 */
template <typename Key, typename Info, typename... Ts>
auto make_seq(Key&& k, Info&& i, Ts&&... vs) -> decltype(auto) {
    return Sequence<std::remove_reference_t<Key>, std::remove_reference_t<Info>>
    { std::forward<Key>(k), std::forward<Info>(i), std::forward<Ts>(vs)... };
}

template <typename Key, typename Info>
auto produce(Sequence<Key, Info> const& a, uint start_a, uint len_a,
             Sequence<Key, Info> const& b, uint start_b, uint len_b, uint limit)
    -> Sequence<Key, Info> {
    auto it_a = a.begin();
    while (start_a--) {
        if (!(++it_a != a.end())) { it_a = a.begin(); }
    }

    auto it_b = b.begin();
    while (start_b--) {
        if(!(++it_b != b.end())) { it_b = b.begin(); }
    }

    auto ret = Sequence<Key, Info> {};
    while (true) {
        for (auto i = 0U; i < len_a; ++i) {
            ret.append(*it_a);
            if (!(++it_a != a.end())) { it_a = a.begin(); }
            if (--limit == 0) { return ret; }
        }

        for (auto i = 0U; i < len_b; ++i) {
            ret.append(*it_b);
            if (!(++it_b != b.end())) { it_b = b.begin(); }
            if (--limit == 0) { return ret; }
        }
    }
}

auto main() -> int {
    produce(make_seq(10, 10, 20, 20, 30, 30, 40, 40, 50, 50, 60, 60, 70, 70, 80, 80),
            2, 2,
            make_seq(1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9),
            1, 3, 11).print();


    {
        auto seq = Sequence<int, int> {};
        assert(seq.empty() && "seq should be empty");

        seq.clear();
        assert(seq.empty() && "seq should be empty");

        seq.insert(2, 3).append(4 ,5).insert(0, 1).append(6, 7);
        assert(!seq.empty() && "seq should not be empty");
        seq.print();

        auto f = seq.popf();
        assert(f.first == 0 && f.second == 1 && "first elem should be { 0, 1 }");

        auto b = seq.popb();
        assert(b.first == 6 && b.second == 7 && "last elem should be { 6, 7 }");

        seq.print();

        seq.insert_at(seq.get_iter_by([] (auto const& e) {
            return e.first == 4;
        }), 10, 30);

        seq.print();

        seq.get_elem_by([] (auto const& e) {
            return e.second == 30;
        }).second = 50;

        seq.print();

        seq.remove_if([] (auto const&) { return true; });
        seq.remove_if([] (auto const&) { return false; });

        seq.print();

        seq.first().print();
        seq.last().print();

        seq.clear();
        seq.append(0, 0);
        seq.popb();
        seq.insert(0, 0);
        seq.popf();
    }
}
