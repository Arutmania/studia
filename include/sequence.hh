#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <type_traits>

#include "util.hh"

/** @file sequence.hpp */

namespace seq {
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
            , next_ { util::make_owning<Node>(std::forward<Ts>(vs)...) }
        {}

        template <typename... Ts>
        Node(Elem const& elem, Ts&&... vs)
            : elem_ { elem }
            , next_ { util::make_owning<Node>(std::forward<Ts>(vs)...) }
        {}

        template <typename... Ts>
        Node(Elem&& elem, Ts&&... vs)
            : elem_ { std::move(elem) }
            , next_ { util::make_owning<Node>(std::forward<Ts>(vs)...) }
        {}

        Node(Node const&) = default;
        Node(Node&&)      = default;

        auto print() const -> void {
            std::cout << elem_.first << ", " << elem_.second;
        }

        auto elem() const -> Elem const& { return elem_; }
        auto elem()       -> Elem&       { return elem_; }

        auto next() const -> util::OwningPtr<Node> const& { return next_; }
        auto next()       -> util::OwningPtr<Node>&       { return next_; }

        private:
        Elem            elem_;
        util::OwningPtr<Node> next_ = nullptr;
    };

    Sequence() = default;

    template <typename T, typename... Ts, typename = std::enable_if_t<
            sizeof...(Ts) != 0 || !std::is_same_v<
                std::remove_reference_t<T>, Sequence
            >
        >
    >
    Sequence(T&& t, Ts&&... vs)
        : head_ {
            util::make_owning<Node>(std::forward<T>(t), std::forward<Ts>(vs)...)
        }
    {}

    Sequence(Sequence const&)                         = default;
    Sequence(Sequence&&) noexcept                     = default;
    auto operator =(Sequence const&)     -> Sequence& = default;
    auto operator =(Sequence&&) noexcept -> Sequence& = default;

    auto empty() const -> bool { return !head_; }

    auto clear() -> void {
        head_.reset();
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
        return *head_;
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
        while (node.get()->next()) {
            node = node.get()->next();
        }
        return *node.get();
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
            seq.last().next() = util::OwningPtr<Node> { head_.release() };
            head_             = util::OwningPtr<Node> { seq.head_.release() };
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
            head_         = seq.head_;
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
        auto ret = head_->elem();
        head_.reset(head_->next().release());
        return ret;
    }

    /**
     * removes and returns last element of the sequence, assserts on empty.
     * @return removed element
     */
    auto popb() -> typename Node::Elem {
        assert(!empty() && "popb on empty sequence");
        auto node = std::ref(head_);
        while (node.get()->next()) {
            node = node.get()->next();
        }
        auto ret = node.get()->elem();
        node.get().reset();
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

        std::reference_wrapper<util::OwningPtr<Node>> elem_;

        auto operator ++() -> Iterator& {
            elem_ = elem_.get()->next();
            return *this;
        }

        auto operator *() const -> reference {
            return elem_.get()->elem();
        }
    };

    auto begin() -> Iterator { return Iterator { head_ }; }

    struct IterEnd {};

    auto end() -> IterEnd { return IterEnd {}; }

    friend auto operator !=(Iterator const& iter, IterEnd const&) -> bool {
        return iter.elem_.get() != nullptr;
    }

    friend auto operator ==(Iterator const& iter, IterEnd const&) -> bool {
        return !(iter != IterEnd {});
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

        std::reference_wrapper<util::OwningPtr<Node> const> elem_;

        auto operator ++() -> ConstIterator& {
            elem_ = elem_.get()->next();
            return *this;
        }

        auto operator *() const -> reference {
            return elem_.get()->elem();
        }
    };

    auto begin() const -> ConstIterator { return ConstIterator { head_ }; }

    auto end() const -> IterEnd { return IterEnd {}; }

    friend auto operator !=(ConstIterator const& iter, IterEnd const&) -> bool {
        return iter.elem_.get() != nullptr;
    }

    friend auto operator ==(ConstIterator const& iter, IterEnd const&) -> bool {
        return !(iter != IterEnd {});
    }

    /**
     * insert node or nodes at position denoted by iterator.
     * @return self
     */
    template <typename... Ts>
    auto insert_at(Iterator const& iter, Ts&&... vs) -> Sequence& {
        auto seq = Sequence { std::forward<Ts>(vs)... };
        seq.last().next() = util::OwningPtr<Node> { iter.elem_.get().release() };
        iter.elem_.get()  = util::OwningPtr<Node> { seq.head_.release() };
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
            it.elem_.get() = it.elem_.get()->next();
        }
        return *this;
    }

private:
    util::OwningPtr<Node> head_ = nullptr;
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
}

// Local Variables:
// flycheck-clang-language-standard: "c++17"
// flycheck-gcc-language-standard:   "c++17"
// End:
