#pragma once

#include "ContextFreeGrammar.hpp"
#include <unordered_map>
#include <string>

namespace FL {

class CYK {
public:
    explicit CYK(ContextFreeGrammar const& grammar);

    ContextFreeGrammar const& grammar() const;
    bool predict(std::string const& word) const;

protected:
    using Table = std::unordered_map<Symbol, std::vector<std::vector<bool>>>;

    bool acceptsEmptyWord() const;
    void findDirectChildren(
        std::unordered_map<Symbol, Alphabet>& terminalChildren,
        std::unordered_map<Symbol, std::vector<Word>>& nonterminalChildren
    ) const;
    Table initTable(
        std::string const& word,
        std::unordered_map<Symbol, Alphabet> const& terminalChildren
    ) const;
    void checkIfNonterminalGeneratesSubword(
        Symbol nonterminal,
        std::unordered_map<Symbol, std::vector<Word>> const& nonterminalChildren,
        Table& generatesSubword,
        size_t subwordStart,
        size_t subwordSize
    ) const;
    Table calculateTableValues(std::string const& word) const;

    ContextFreeGrammar _grammar;
};

}
