#include <algorithm>
#include <iostream>
#include <vector>

#include "../include/ring.hh"
#include "../include/util.hh"

template <template <typename...> typename Container, typename... Ts>
auto operator <<(std::ostream& os, Container<Ts...> const& v) -> std::ostream& {
    for (auto const& e : v) { os << e << ", "; }
    return os;
}

/**
 * every second to res1 and other to res2 from source.
 * taken in counterclockwise dir.
 * the dir parameters say in which direction are elements added
 * to the destinations.
 *
 * EX:
 *  source = v-----------------------v
 *           1 <-> 2 <-> 3 <-> 4 <-> 5
 *           ^     ^- r2 should start from here
 *           |
 *           r1 should start from here
 *
 *  split(source, r1, true, 3, r2, false, 6) =>
 *
 *  r1     = v-----------v
 *           1 <-> 3 <-> 5                              // take in clockwise dir
 *           ^- start with firt element clockwise       // len 3
 *
 *  r2     = v-----------------------------v
 *           2 <-> 2 <-> 5 <-> 3 <-> 1 <-> 4            // take counterclockwise
 *           ^- start with second element clockwise     // len 6
 *
 * clockwise basically means append at the end and counterclockwise in front
 */
template <typename Key>
auto split(Ring<Key> const& source,
        Ring<Key>& result1, Direction dir1, int len1,
        Ring<Key>& result2, Direction dir2, int len2) -> void {
    result1.clear();
    result2.clear();

    if (source.empty()) { return; }
    auto iter = util::Repeat<Ring<int>::ConstIterator> {
        source.begin(), source.end()
    };

    while (len1 > 0 || len2 > 0) {
        if (len1-- > 0) { result1.insert(*iter, dir1); }
        ++iter;
        if (len2-- > 0) { result2.insert(*iter, dir2); }
        ++iter;
    }
}

auto main() -> int {
    // split
    {
        std::cout << "split\n\n";
        auto source = Ring<int> ({ 1, 2, 3, 4, 5 });
        auto result1 = Ring<int> {};
        auto result2 = Ring<int> {};

        split(source,
              result1, Direction::Front, 3,
              result2, Direction::Back, 6);

        std::cout << "source:  " << source  << '\n'
                  << "result1: " << result1 << " (dir : Direction::Front, len : 3)\n"
                  << "result2: " << result2 << " (dir : Direction::Back, len : 6)\n";
    }

    // stl
    {
        std::cout << "\n\nSTL algos\n\n";
        auto ring = Ring<int> ({ 1, 2, 3, 4, 5 });
        auto vec = std::vector<int> {};

        std::copy(ring.begin(), ring.end(), std::back_inserter(vec));

        std::cout << "ring: " << ring << '\n'
                  << "vec:  " << vec  << '\n';
    }

    {
        std::cout << "\n\nmisc. opreations on rings\n\n";
        auto r = Ring<int> {};
        auto s = Ring<int> ({ 1, 2, 3 });



        std::cout << "r: " << r << '\n'
                  << "s: " << s << '\n';

        r.insert(1, Direction::Front);
        r.insert(2, Direction::Back);
        std::cout << "r: " << r << '\n';

        r.insert({ 3, 4, 5 }, Direction::Front);
        std::cout << "r: " << r << '\n';

        r.rotate(r.find([] (Ring<int>::Iterator& it) { return (*it) == 3; }));
        std::cout << "r: " << r << '\n';

        auto copy = Ring<int> { r };
        std::cout << "copy: " << copy << '\n';

        auto move = Ring<int> { Ring<int> ({ 1, 2, 3 }) };
        std::cout << "move: " << move << '\n';
    }

    // single node
    {
        std::cout << "\n\ntests on rings containing only one or zero nodes\n\n";
        auto single = Ring<int> ({ 99 });
        auto val = single.pop(single.first());
        std::cout << "single: " << single << '\n'
                  << "val:    " << val << '\n';
    }
}
