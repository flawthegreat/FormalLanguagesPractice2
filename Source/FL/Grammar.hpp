#pragma once

#include "Common.hpp"
#include "Constants.hpp"
#include <vector>
#include <string>
#include <exception>

namespace FL {

class Grammar {
public:
    struct Rule {
        Word lhs;
        Word rhs;

        Rule(Word const& lhs, Word const& rhs);
        Rule(std::string const& lhs, std::string const& rhs);

        bool operator==(Rule const& rule) const;
        bool operator!=(Rule const& rule) const;
    };

    Grammar(
        Alphabet const& terminals,
        Alphabet const& nonterminals,
        Symbol startSymbol,
        std::vector<Rule> const& rules
    );

    Alphabet const& terminals() const;
    Alphabet const& nonterminals() const;
    Symbol startSymbol() const;
    std::vector<Rule> const& rules() const;

    bool isContextFree() const;

    bool symbolIsTerminal(Symbol const& symbol) const;
    bool symbolIsNonterminal(Symbol const& symbol) const;
    bool symbolIsCorrect(Symbol const& symbol) const;
    bool ruleIsCorrect(Rule const& rule) const;

protected:
    bool isCorrect() const;

    Symbol addNewTerminal();
    Symbol addNewNonterminal();

    Alphabet _terminals;
    Alphabet _nonterminals;
    Symbol _startSymbol;
    std::vector<Rule> _rules;
    Symbol _maxSymbol;
};

struct IncorrectGrammarException: std::exception {
    char const* what() const throw();
};

struct GrammarOutOfSymbolsException: std::exception {
    char const* what() const throw();
};

}
