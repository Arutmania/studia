#include <functional>

#include "../include/sequence.hpp"

template <typename Key, typename Info>
auto produce(seq::Sequence<Key, Info> const& a, uint start_a, uint len_a,
             seq::Sequence<Key, Info> const& b, uint start_b, uint len_b, uint limit)
    -> seq::Sequence<Key, Info> {


    auto ita = util::Repeat(a.begin(), a.end());
    std::advance(ita, start_a);

    auto itb = util::Repeat(b.begin(), b.end());
    std::advance(itb, start_b);

    auto ret = seq::Sequence<Key, Info> {};

    while (true) {
        for (auto i = 0U; i < len_a; ++i) {
            ret.append(*ita++);
            if (--limit == 0) { return ret; }
        }

        for (auto i = 0U; i < len_b; ++i) {
            ret.append(*itb++);
            if (--limit == 0) { return ret; }
        }
    }
}

auto main() -> int {
    auto left = seq::make_seq(
        10, 10,
        20, 20,
        30, 30,
        40, 40,
        50, 50,
        60, 60,
        70, 70,
        80, 80
    );

    auto right = seq::make_seq(
        1, 1,
        2, 2,
        3, 3,
        4, 4,
        5, 5,
        6, 6,
        7, 7,
        8, 8,
        9, 9
    );

    produce(left, 2, 2, right, 1, 3, 11).print();


    {
        auto seq = seq::Sequence<int, int> {};
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

        auto accept = [] (auto const&) {
            return true;
        };

        auto decline = std::not_fn(accept);

        seq.remove_if(accept);
        seq.remove_if(decline);

        seq.print();

        seq.first().print();
        std::cout << '\n';

        seq.last().print();
        std::cout << '\n';

        seq.clear();
        seq.append(0, 0);
        seq.popb();
        seq.insert(0, 0);
        seq.popf();

        seq.insert(0, 0).insert(1, 1).insert(2, 2);

        auto copy = seq::Sequence<int, int> { seq };
        copy.print();

        auto move = seq::Sequence<int, int> { std::move(copy) };
        move.print();
    }
}


// Local Variables:
// flycheck-clang-language-standard: "c++17"
// flycheck-gcc-language-standard:   "c++17"
// End:
