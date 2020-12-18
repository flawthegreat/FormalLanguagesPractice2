#pragma once

#include "Grammar.hpp"
#include <unordered_set>

namespace FL {

class ContextFreeGrammar: public Grammar {
public:
    ContextFreeGrammar(Grammar const& grammar);
    ContextFreeGrammar(
        Alphabet const& terminals,
        Alphabet const& nonterminals,
        Symbol startSymbol,
        std::vector<Rule> const& rules
    );

    bool isNormalized() const;
    void normalize();
    ContextFreeGrammar normalized() const;

protected:
    bool hasLongRules() const;
    void removeLongRules();
    std::unordered_set<Symbol> findEpsilonGenerators() const;
    void removeEmptyRules();
    std::vector<std::pair<Symbol, Symbol>> findChainedPairs() const;
    void removeChainRules();
    std::unordered_set<Symbol> findGeneratingNonterminals() const;
    void removeNonGeneratingRules();
    std::unordered_set<Symbol> findReachableNonterminals() const;
    void removeNonReachableRules();
    void removeMixedRules();
};

struct NonContextFreeGrammarException: std::exception {
    char const* what() const throw();
};

struct FoundLongRuleException: std::exception {
    char const* what() const throw();
};

}
