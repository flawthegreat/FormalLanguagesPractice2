#pragma once

#include <unordered_set>
#include <vector>
#include <limits>

namespace FL {

struct Symbol {
    static constexpr int maxValue = std::numeric_limits<int>::max();

    int rawValue = 0;

    Symbol(int rawValue);
    Symbol(char character);

    bool operator==(Symbol const& symbol) const;
    bool operator!=(Symbol const& symbol) const;
};

using Alphabet = std::unordered_set<Symbol>;
using Word = std::vector<Symbol>;

}

namespace std {

template<>
struct hash<FL::Symbol> {
    size_t operator()(const FL::Symbol& symbol) const;
};

}
