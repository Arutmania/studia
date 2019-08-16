#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

#include "util.hh"

enum struct Direction : bool {
    Front = true,
    Back  = false,
};

template <typename Key>
struct Ring {
    using size_type = std::size_t;

    private:
    struct Node {
        Key   key;
        Node* next;
        Node* prev;

        Node(Key const& k, Node* n, Node* p)
            : key  ( k )
            , next { n }
            , prev { p }
        {}

        Node(Key&& k, Node* n, Node* p)
            : key  ( std::move(k) )
            , next { n }
            , prev { p }
        {}

        Node(Key const& k) : Node { k, this, this } {}
        Node(Key&& k) : Node { std::move(k), this, this } {}

        auto pop() -> Node* {
            prev->next = next;
            next->prev = prev;
            return this;
        }

        /*
         * return pointer to first inserted node
         */
        auto insert(Node* ns, Direction dir) -> Node* {
            switch (dir) {
                case Direction::Front: {
                    std::swap(ns->prev->next, prev->next);
                    std::swap(prev, ns->prev);
                    break;
                }
                case Direction::Back: {
                    std::swap(next, ns->prev->next);
                    std::swap(ns->prev, next->prev);
                    break;
                }
            }
            return ns;
        }

        /**
         * create ring of nodes from array of keys
         * @return first node
         */
        template <size_type N>
        static auto from(Key const (&keys)[N]) -> Node* {
            auto* ret = new Node { keys[0] };
            for (auto it = std::begin(keys) + 1; it != std::end(keys); ++it) {
                ret->insert(new Node { *it }, Direction::Front);
            }
            return ret;
        }

        auto copy() const -> Node* {
            auto it = begin();
            auto* ret = new Node { (*it++).key };
            for (; it != end(); ++it) {
                ret->insert(new Node { (*it).key }, Direction::Front);
            }
            return ret;
        }

        template <template <typename> typename Transform>
        struct IteratorImpl {
            using difference_type   = size_type;
            using value_type        = typename Transform<Node>::type;
            using pointer           = value_type*;
            using reference         = value_type&;
            using iterator_category = std::bidirectional_iterator_tag;

            pointer current;
            pointer first;

            IteratorImpl(pointer p) : current { p }, first { p } {}

            auto operator ==(IteratorImpl const& rhs) const -> bool {
                return current == rhs.current;
            }

            auto operator !=(IteratorImpl const& rhs) const -> bool {
                return current != rhs.current;
            }

            auto operator ++() -> IteratorImpl& {
                current = current->next != first ? current->next : nullptr;
                return *this;
            }

            auto operator ++(int) -> IteratorImpl {
                auto ret = *this;
                ++*this;
                return ret;
            }

            auto operator --() -> IteratorImpl& {
                current = current ? current->prev : first->prev;
                return *this;
            }

            auto operator --(int) -> IteratorImpl {
                auto ret = *this;
                ++*this;
                return ret;
            }

            auto operator *() const -> reference { return *current; }

        };

        using Iterator      = IteratorImpl<util::type_identity>;
        using ConstIterator = IteratorImpl<std::add_const>;

        auto begin() const & -> ConstIterator { return ConstIterator { this }; }
        auto begin() & -> Iterator { return Iterator { this }; }

        // is iterating over temporary (&&) possible
        auto end() & -> Iterator { return Iterator { nullptr }; }
        auto end() const & -> ConstIterator { return ConstIterator { nullptr }; }
    };

    public:
    template <template <typename> typename Transform>
    struct IteratorImpl {
        using difference_type   = size_type;
        using value_type        = typename Transform<Key>::type;
        using pointer           = value_type*;
        using reference         = value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

        IteratorImpl(typename Node::template IteratorImpl<Transform>::pointer p)
            : inner { p }
        {}

        auto operator ==(IteratorImpl const& rhs) const -> bool {
            return inner == rhs.inner;
        }

        auto operator !=(IteratorImpl const& rhs) const -> bool {
            return inner != rhs.inner;
        }

        auto operator ++() -> IteratorImpl& { return ++inner, *this; }
        auto operator --() -> IteratorImpl& { return --inner, *this; }

        auto operator *() const -> reference { return (*inner).key; }

        friend class Ring;
        private:
        typename Node::template IteratorImpl<Transform> inner;
    };

    using Iterator      = IteratorImpl<util::type_identity>;
    using ConstIterator = IteratorImpl<std::add_const>;

    auto begin() & -> Iterator { return Iterator { first_ }; }
    auto begin() const & -> ConstIterator {
        return ConstIterator { first_ };
    }
    auto end() & -> Iterator { return Iterator { nullptr }; }
    auto end() const & -> ConstIterator {
        return ConstIterator { nullptr };
    }

    Ring() = default;
    ~Ring() {
        clear();
    }

    Ring(Ring const& other)
        : first_ { other.first_ ? other.first_->copy() : nullptr } {}

    Ring(Ring&& other) : first_ { other.first_ } {
        other.first_ = nullptr;
    }

    auto operator =(Ring other) -> Ring& {
        std::swap(first_, other.first_);
        return *this;
    }

    auto operator =(Ring&& other) -> Ring& {
        std::swap(first_, other.first_);
        return *this;
    }

    template <size_type N>
    Ring(Key const (&keys)[N]) : first_ { Node::from(keys) } {}

    auto empty() const -> bool { return first_ == nullptr; }

    auto clear() -> void {
        if (!first_) { return; }
        auto* current = first_;
        do {
            auto* del = current;
            current = current->next;
            delete del;
        } while (current != first_);
        first_ = nullptr;
    }

    auto first() & -> Iterator { return Iterator { first_ }; }
    auto first() const & -> ConstIterator {
        return ConstIterator { first_ };
    }
    auto last() & -> Iterator { return Iterator { first_->prev }; }
    auto last() const & -> ConstIterator {
        return ConstIterator { first_->prev };
    }

    /**
     * find node with key given by predicate f, return iterator to it
     * @return iterator given by predicate f
     */
    template <typename F>
    auto find(F const& f) -> Iterator {
        auto it = begin();
        for (; it != end(); ++it) {
            if (f(it)) { break; }
        }
        return it;
    }

    template <typename F>
    auto find(F const& f) const -> ConstIterator {
        auto it = begin();
        for (; it != end(); ++it) {
            if (f(it)) { break; }
        }
        return it;
    }

    auto insert(Key const& k, Direction dir) -> Iterator {
        if (empty()) {
            first_ = new Node { k };
            return Iterator { first_ };
        } else {
            return Iterator { first_->insert(new Node { k }, dir) };
        }
    }

    template <size_type N>
    auto insert(Key const (&keys)[N], Direction dir) -> Iterator {
        if (empty()) {
            first_ = Node::from(keys);
            return Iterator { first_ };
        } else {
            return Iterator { first_->insert(Node::from(keys), dir) };
        }
    }

    auto insert(Ring const& other, Direction dir) -> Iterator {
        if (other.empty()) { return first(); }
        auto* copy = other.first_.copy();
        if (empty()) {
            return Iterator { first_ = copy };
        } else {
            return Iterator { first_->insert(copy, dir) };
        }
    }

    /**
     * insert node with key at position pos in direction dir
     * @return iterator at inserted node
     */
    static auto insert_at(Iterator const& pos, Key const& k, Direction dir)
        -> Iterator {
        return Iterator { pos.inner.current->insert(k, dir) };
    }

    template <size_type N>
    static auto insert_at(Iterator const& pos, Key const (&keys)[N], Direction dir)
        -> Iterator {
        return Iterator { pos.inner.current->inssert(from(keys), dir) };
    }

    static auto insert_at(Iterator const& pos, Ring const& other, Direction dir)
        -> Iterator {
        if (other.empty()) { return pos; } else {
            return Iterator {
                pos.inner.current->insert(other.first_->copy(), dir)
            };
        }
    }

    /**
     * move the element to which ring points to pos.
     * if pos points to a different ring rotate can be used to
     * exchange elements between rings. if used inappropriately
     * can lead to double free.
     */
    auto rotate(Iterator const& pos) -> Iterator {
        auto ret = Iterator { first_ };
        first_ = pos.inner.current;
        return ret;
    }

    static auto pop(Iterator const& pos) -> Key {
        auto* del = pos.inner.current->pop();
        auto ret = del->key;
        delete del;
        return ret;
    }

    static auto swap(Ring& a, Ring& b) -> void {
        std::swap(a.first_, b.first_);
    }

    private:
    Node* first_ = {};
};
