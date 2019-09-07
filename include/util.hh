#pragma once

#include <iterator>
#include <memory>
#include <utility>

namespace util {
template <typename T>
struct type_identity {
    using type = T;
};

/**
 * @class OwningPtr
 * std::unique_ptr wrapper allowing for copying owned value
 */
template <typename T, typename D = typename std::unique_ptr<T>::deleter_type>
struct OwningPtr : std::unique_ptr<T, D> {
    using std::unique_ptr<T, D>::unique_ptr;

    OwningPtr(std::unique_ptr<T, D> const& other)
        : std::unique_ptr<T, D> { other ? new T(*other) : nullptr } {}

    OwningPtr(OwningPtr const& other)
        : std::unique_ptr<T, D> { other ? new T(*other) : nullptr } {}

    auto operator =(std::unique_ptr<T, D> const& rhs) -> OwningPtr& {
        this->reset(rhs ? new T(*rhs) : nullptr);
        return *this;
    }

    auto operator =(OwningPtr const& rhs) -> OwningPtr& {
        this->reset(rhs ? new T(*rhs) : nullptr);
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
    return OwningPtr<T> { new T(std::forward<Args>(args)... ) };
}

/**
 * @class Repeat
 * looping iterator wrapper
 */
template <typename Beg, typename End = Beg>
struct Repeat {
    Beg beginning_;
    End end_;
    Beg current_;

    Repeat(Beg const& beginning, End const& end)
        : beginning_(beginning)
        , end_(end)
        , current_(beginning) {}

    using iterator_category = std::forward_iterator_tag;

    using value_type        = typename Beg::value_type;
    using difference_type   = typename Beg::difference_type;
    using pointer           = typename Beg::pointer;
    using reference         = typename Beg::reference;

    auto operator ++() -> Repeat& {
        if (++current_ == end_) { current_ = beginning_; }
        return *this;
    }

    auto operator ++(int) -> Repeat {
        auto ret = *this;
        ++*this;
        return ret;
    }

    auto operator *() const -> reference {
        return *current_;
    }

    template <typename T>
    auto operator !=(T&&) -> bool { return true; }
    template <typename T>
    auto operator ==(T&&) -> bool { return false; }
};
}
