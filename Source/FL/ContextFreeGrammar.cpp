#include "ContextFreeGrammar.hpp"

#include <algorithm>
#include <unordered_map>

namespace FL {

ContextFreeGrammar::ContextFreeGrammar(Grammar const& grammar):
    Grammar(grammar)
{
    if (!isContextFree()) {
        throw NonContextFreeGrammarException();
    }
}

ContextFreeGrammar::ContextFreeGrammar(
    Alphabet const& terminals,
    Alphabet const& nonterminals,
    Symbol startSymbol,
    std::vector<Rule> const& rules
):
    Grammar(terminals, nonterminals, startSymbol, rules)
{
    if (!isContextFree()) {
        throw NonContextFreeGrammarException();
    }
}

bool ContextFreeGrammar::isNormalized() const {
    for (auto const& [lhs, rhs]: _rules) {
        if (
            (rhs.size() > 2) ||
            (rhs.size() == 1 && symbolIsNonterminal(rhs[0])) ||
            (rhs.size() == 2 && (
                symbolIsTerminal(rhs[0]) || symbolIsTerminal(rhs[1]) ||
                rhs[0] == _startSymbol || rhs[1] == _startSymbol
            )) ||
            (rhs.empty() && lhs[0] != _startSymbol)
        ) {
            return false;
        }
    }

    return true;
}

void ContextFreeGrammar::normalize() {
    if (isNormalized()) {
        return;
    }

    removeLongRules();
    removeEmptyRules();
    removeChainRules();
    removeNonGeneratingRules();
    removeNonReachableRules();
    removeMixedRules();
}

ContextFreeGrammar ContextFreeGrammar::normalized() const {
    auto copy = *this;
    copy.normalize();
    return copy;
}

bool ContextFreeGrammar::hasLongRules() const {
    for (auto const& [lhs, rhs]: _rules) {
        if (rhs.size() > 2) {
            return true;
        }
    }
    return false;
}

void ContextFreeGrammar::removeLongRules() {
    for (int i = static_cast<int>(_rules.size()) - 1; i >= 0; --i) {
        auto [lhs, rhs] = _rules[i];
        if (rhs.size() <= 2) {
            continue;
        }

        _rules.emplace_back(lhs, Word{rhs[0], addNewNonterminal()});
        for (size_t j = 1; j < rhs.size() - 2; ++j) {
            _rules.emplace_back(Word{_rules.back().rhs[1]}, Word{rhs[j], addNewNonterminal()});
        }
        _rules.emplace_back(Word{_rules.back().rhs[1]}, Word{rhs[rhs.size() - 2], rhs.back()});

        _rules.erase(_rules.begin() + i);
    }
}

std::unordered_set<Symbol> ContextFreeGrammar::findEpsilonGenerators() const {
    if (hasLongRules()) {
        throw FoundLongRuleException();
    }

    std::unordered_set<Symbol> generators;
    for (auto const& [lhs, rhs]: _rules) {
        if (rhs.empty()) {
            generators.insert(lhs[0]);
        }
    }

    for (size_t setSize = generators.size();; setSize = generators.size()) {
        for (auto const& [lhs, rhs]: _rules) {
            if (rhs.empty()) {
                continue;
            }
            if (
                (rhs.size() == 2 && generators.count(rhs[0]) && generators.count(rhs[1])) ||
                (rhs.size() == 1 && generators.count(rhs[0]))
            ) {
                generators.insert(lhs[0]);
            }
        }
        if (setSize == generators.size()) {
            break;
        }
    }

    return generators;
}

void ContextFreeGrammar::removeEmptyRules() {
    if (hasLongRules()) {
        throw FoundLongRuleException();
    }

    auto epsilonGenerators = findEpsilonGenerators();
    for (int i = static_cast<int>(_rules.size()) - 1; i >= 0; --i) {
        auto [lhs, rhs] = _rules[i];
        if (rhs.empty()) {
            _rules.erase(_rules.begin() + i);
        }
        if (rhs.size() < 2) {
            continue;
        }
        for (size_t j = 0; j < 2; ++j) {
            if (epsilonGenerators.count(rhs[j])) {
                _rules.emplace_back(lhs, Word{rhs[1 - j]});
            }
        }
    }

    if (epsilonGenerators.count(_startSymbol)) {
        auto newStartSymbol = addNewNonterminal();
        _rules.emplace_back(Word{newStartSymbol}, emptyWord);
        _rules.emplace_back(Word{newStartSymbol}, Word{_startSymbol});
        _startSymbol = newStartSymbol;
    }
}

std::vector<std::pair<Symbol, Symbol>> ContextFreeGrammar::findChainedPairs() const {
    std::vector<std::pair<Symbol, Symbol>> pairs;
    std::unordered_map<Symbol, std::unordered_map<Symbol, bool>> isChainedPair;
    for (auto const& [lhs, rhs]: _rules) {
        if (rhs.size() == 1 && symbolIsNonterminal(rhs[0])) {
            pairs.emplace_back(lhs[0], rhs[0]);
            isChainedPair[lhs[0]][rhs[0]] = true;
        }
    }
    for (bool foundNewPairs = true; foundNewPairs;) {
        foundNewPairs = false;
        for (auto const& [lhs, rhs]: _rules) {
            if (rhs.size() != 1 || !symbolIsNonterminal(rhs[0])) {
                continue;
            }
            for (int i = static_cast<int>(pairs.size()) - 1; i >= 0; --i) {
                if (pairs[i].second != lhs[0] || isChainedPair[pairs[i].first][rhs[0]]) {
                    continue;
                }
                isChainedPair[pairs[i].first][rhs[0]] = true;
                pairs.emplace_back(pairs[i].first, rhs[0]);
                foundNewPairs = true;
            }
        }
    }

    return pairs;
}

void ContextFreeGrammar::removeChainRules() {
    auto chainedPairs = findChainedPairs();
    for (int i = static_cast<int>(_rules.size()) - 1; i >= 0; --i) {
        if (_rules[i].rhs.size() == 1 && symbolIsNonterminal(_rules[i].rhs[0])) {
            _rules.erase(_rules.begin() + i);
        }
    }
    for (auto const& [start, end]: chainedPairs) {
        for (int i = static_cast<int>(_rules.size()) - 1; i >= 0; --i) {
            auto [lhs, rhs] = _rules[i];
            if (lhs[0] == end) {
                _rules.emplace_back(Word{start}, rhs);
            }
        }
    }
}

std::unordered_set<Symbol> ContextFreeGrammar::findGeneratingNonterminals() const {
    if (hasLongRules()) {
        throw FoundLongRuleException();
    }

    std::unordered_set<Symbol> nonterminals;
    for (auto const& [lhs, rhs]: _rules) {
        if (
            (rhs.empty()) ||
            (rhs.size() == 1 && symbolIsTerminal(rhs[0])) ||
            (rhs.size() == 2 && symbolIsTerminal(rhs[0]) && symbolIsTerminal(rhs[1]))
        ) {
            nonterminals.insert(lhs[0]);
        }
    }

    for (size_t setSize = nonterminals.size();; setSize = nonterminals.size()) {
        for (auto const& [lhs, rhs]: _rules) {
            bool allRHSNonterminalsAreGenerating = true;
            for (auto symbol: rhs) {
                if (symbolIsNonterminal(symbol) && !nonterminals.count(symbol)) {
                    allRHSNonterminalsAreGenerating = false;
                    break;
                }
            }
            if (allRHSNonterminalsAreGenerating) {
                nonterminals.insert(lhs[0]);
            }
        }
        if (setSize == nonterminals.size()) {
            break;
        }
    }

    return nonterminals;
}

void ContextFreeGrammar::removeNonGeneratingRules() {
    auto generatingNonterminals = findGeneratingNonterminals();
    for (int i = static_cast<int>(_rules.size()) - 1; i >= 0; --i) {
        auto [lhs, rhs] = _rules[i];
        bool shouldBeRemoved = !generatingNonterminals.count(lhs[0]);
        for (auto symbol: rhs) {
            if (symbolIsNonterminal(symbol) && !generatingNonterminals.count(symbol)) {
                shouldBeRemoved = true;
                break;
            }
        }
        if (shouldBeRemoved) {
            _rules.erase(_rules.begin() + i);
        }
    }
}

std::unordered_set<Symbol> ContextFreeGrammar::findReachableNonterminals() const {
    std::unordered_set<Symbol> nonterminals = {_startSymbol};
    for (size_t setSize = nonterminals.size();; setSize = nonterminals.size()) {
        for (auto const& [lhs, rhs]: _rules) {
            if (!nonterminals.count(lhs[0])) {
                continue;
            }
            for (auto symbol: rhs) {
                if (symbolIsNonterminal(symbol)) {
                    nonterminals.insert(symbol);
                }
            }
        }
        if (setSize == nonterminals.size()) {
            break;
        }
    }

    return nonterminals;
}

void ContextFreeGrammar::removeNonReachableRules() {
    auto reachableNonterminals = findReachableNonterminals();
    for (int i = static_cast<int>(_rules.size()) - 1; i >= 0; --i) {
        auto [lhs, rhs] = _rules[i];
        bool shouldBeRemoved = !reachableNonterminals.count(lhs[0]);
        for (auto symbol: rhs) {
            if (symbolIsNonterminal(symbol) && !reachableNonterminals.count(symbol)) {
                shouldBeRemoved = true;
                break;
            }
        }
        if (shouldBeRemoved) {
            _rules.erase(_rules.begin() + i);
        }
    }
}

void ContextFreeGrammar::removeMixedRules() {
    if (hasLongRules()) {
        throw FoundLongRuleException();
    }

    for (int i = static_cast<int>(_rules.size()) - 1; i >= 0; --i) {
        auto [lhs, rhs] = _rules[i];
        if (rhs.size() < 2 || (symbolIsNonterminal(rhs[0]) && symbolIsNonterminal(rhs[1]))) {
            continue;
        }
        if (symbolIsNonterminal(rhs[0]) && symbolIsTerminal(rhs[1])) {
            _rules.emplace_back(lhs, Word{rhs[0], addNewNonterminal()});
            _rules.emplace_back(Word{_rules.back().rhs[1]}, Word{rhs[1]});
        } else if (symbolIsTerminal(rhs[0]) && symbolIsNonterminal(rhs[1])) {
            _rules.emplace_back(lhs, Word{addNewNonterminal(), rhs[1]});
            _rules.emplace_back(Word{_rules.back().rhs[0]}, Word{rhs[0]});
        } else {
            Rule newRule(lhs, Word{addNewNonterminal(), addNewNonterminal()});
            _rules.emplace_back(Word{newRule.rhs[0]}, Word{rhs[0]});
            _rules.emplace_back(Word{newRule.rhs[1]}, Word{rhs[1]});
            _rules.emplace_back(std::move(newRule));
        }
        _rules.erase(_rules.begin() + i);
    }
}

char const* NonContextFreeGrammarException::what() const throw() {
    return "Grammar is not context-free";
}

char const* FoundLongRuleException::what() const throw() {
    return "This function expects only short rules";
}

}
