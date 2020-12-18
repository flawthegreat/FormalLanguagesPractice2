#include "CYK.hpp"

#include <unordered_map>
#include <iostream>

namespace FL {

CYK::CYK(ContextFreeGrammar const& grammar):
    _grammar(grammar.normalized())
{}

ContextFreeGrammar const& CYK::grammar() const {
    return _grammar;
}

bool CYK::predict(std::string const& word) const {
    if (word.empty()) {
        return acceptsEmptyWord();
    }

    auto generatesSubword = calculateTableValues(word);
    return generatesSubword[_grammar.startSymbol()][0][word.size() - 1];
}

bool CYK::acceptsEmptyWord() const {
    for (auto const& [lhs, rhs]: _grammar.rules()) {
        if (lhs[0] == _grammar.startSymbol() && rhs.empty()) {
            return true;
        }
    }
    return false;
}

void CYK::findDirectChildren(
    std::unordered_map<Symbol, Alphabet>& terminalChildren,
    std::unordered_map<Symbol, std::vector<Word>>& nonterminalChildren
) const {
    for (auto nonterminal: _grammar.nonterminals()) {
        terminalChildren[nonterminal] = {};
        nonterminalChildren[nonterminal] = {};
    }

    for (auto const& [lhs, rhs]: _grammar.rules()) {
        if (rhs.size() == 1 && _grammar.symbolIsTerminal(rhs[0])) {
            terminalChildren[lhs[0]].insert(rhs[0]);
        } else if (rhs.size() == 2) {
            nonterminalChildren[lhs[0]].emplace_back(rhs);
        }
    }
}

CYK::Table CYK::initTable(
    std::string const& word,
    std::unordered_map<Symbol, Alphabet> const& terminalChildren
) const {
    Table generatesSubword;
    for (auto nonterminal: _grammar.nonterminals()) {
        generatesSubword[nonterminal] = std::vector<std::vector<bool>>(
            word.size(),
            std::vector<bool>(word.size())
        );
        for (size_t i = 0; i < word.size(); ++i) {
            generatesSubword[nonterminal][i][i] = terminalChildren.at(nonterminal).count(word[i]);
        }
    }

    return generatesSubword;
}

void CYK::checkIfNonterminalGeneratesSubword(
    Symbol nonterminal,
    std::unordered_map<Symbol, std::vector<Word>> const& nonterminalChildren,
    Table& generatesSubword,
    size_t subwordStart,
    size_t subwordSize
) const {
    bool canGenerate = false;
    for (auto const& rhs: nonterminalChildren.at(nonterminal)) {
        for (size_t i = subwordStart; i + 1 < subwordStart + subwordSize; ++i) {
            canGenerate |= (
                generatesSubword[rhs[0]][subwordStart][i] &&
                generatesSubword[rhs[1]][i + 1][subwordStart + subwordSize - 1]
            );
        }
    }
    generatesSubword[nonterminal][subwordStart][subwordStart + subwordSize - 1] = canGenerate;
}

CYK::Table CYK::calculateTableValues(std::string const& word) const {
    std::unordered_map<Symbol, Alphabet> terminalChildren;
    std::unordered_map<Symbol, std::vector<Word>> nonterminalChildren;
    findDirectChildren(terminalChildren, nonterminalChildren);
    auto generatesSubword = initTable(word, terminalChildren);

    for (size_t subwordSize = 2; subwordSize <= word.size(); ++subwordSize) {
        for (size_t subwordStart = 0; subwordStart + subwordSize <= word.size(); ++subwordStart) {
            for (auto nonterminal: _grammar.nonterminals()) {
                checkIfNonterminalGeneratesSubword(
                    nonterminal,
                    nonterminalChildren,
                    generatesSubword,
                    subwordStart,
                    subwordSize
                );
            }
        }
    }

    return generatesSubword;
}

}
