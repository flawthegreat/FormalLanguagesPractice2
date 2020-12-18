#include "Grammar.hpp"

namespace FL {

Grammar::Grammar(
    Alphabet const& terminals,
    Alphabet const& nonterminals,
    Symbol startSymbol,
    std::vector<Rule> const& rules
):
    _terminals(terminals),
    _nonterminals(nonterminals),
    _startSymbol(startSymbol),
    _rules(rules),
    _maxSymbol(0)
{
    if (!isCorrect()) {
        throw IncorrectGrammarException();
    }

    for (auto symbol: _terminals) {
        _maxSymbol.rawValue = std::max(_maxSymbol.rawValue, symbol.rawValue);
    }
    for (auto symbol: _nonterminals) {
        _maxSymbol.rawValue = std::max(_maxSymbol.rawValue, symbol.rawValue);
    }
}

Alphabet const& Grammar::terminals() const {
    return _terminals;
}

Alphabet const& Grammar::nonterminals() const {
    return _nonterminals;
}

Symbol Grammar::startSymbol() const {
    return _startSymbol;
}

std::vector<Grammar::Rule> const& Grammar::rules() const {
    return _rules;
}

bool Grammar::isContextFree() const {
    for (auto const& [lhs, rhs]: _rules) {
        if (lhs.size() > 1 || (lhs.size() == 1 && !symbolIsNonterminal(lhs[0]))) {
            return false;
        }
    }
    return true;
}

bool Grammar::symbolIsTerminal(Symbol const& symbol) const {
    return _terminals.count(symbol);
}

bool Grammar::symbolIsNonterminal(Symbol const& symbol) const {
    return _nonterminals.count(symbol);
}

bool Grammar::symbolIsCorrect(Symbol const& symbol) const {
    return symbolIsTerminal(symbol) || symbolIsNonterminal(symbol);
}

bool Grammar::ruleIsCorrect(Rule const& rule) const {
    if (rule.lhs.empty()) {
        return false;
    }

    for (auto symbol: rule.rhs) {
        if (!symbolIsCorrect(symbol)) {
            return false;
        }
    }

    bool lhsHasNonterminals = false;
    for (auto symbol: rule.lhs) {
        if (!symbolIsCorrect(symbol)) {
            return false;
        }
        if (symbolIsNonterminal(symbol)) {
            lhsHasNonterminals = true;
        }
    }

    return lhsHasNonterminals;
}

bool Grammar::isCorrect() const {
    if (!symbolIsNonterminal(_startSymbol)) {
        return false;
    }

    for (auto symbol: _terminals) {
        if (symbolIsNonterminal(symbol)) {
            return false;
        }
    }

    for (auto const& rule: _rules) {
        if (!ruleIsCorrect(rule)) {
            return false;
        }
    }

    return true;
}

Symbol Grammar::addNewTerminal() {
    if (_maxSymbol.rawValue == Symbol::maxValue) {
        throw GrammarOutOfSymbolsException();
    }
    _terminals.emplace(++_maxSymbol.rawValue);
    return _maxSymbol;
}

Symbol Grammar::addNewNonterminal() {
    if (_maxSymbol.rawValue == Symbol::maxValue) {
        throw GrammarOutOfSymbolsException();
    }
    _nonterminals.emplace(++_maxSymbol.rawValue);
    return _maxSymbol;
}

Grammar::Rule::Rule(Word const& lhs, Word const& rhs):
    lhs(lhs),
    rhs(rhs)
{}

Grammar::Rule::Rule(std::string const& lhs, std::string const& rhs) {
    for (auto character: lhs) {
        this->lhs.emplace_back(character);
    }
    for (auto character: rhs) {
        this->rhs.emplace_back(character);
    }
}

bool Grammar::Rule::operator==(Rule const& rule) const {
    if (lhs.size() != rule.lhs.size() || rhs.size() != rule.rhs.size()) {
        return false;
    }

    for (size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rule.lhs[i]) {
            return false;
        }
    }

    for (size_t i = 0; i < rhs.size(); ++i) {
        if (rhs[i] != rule.rhs[i]) {
            return false;
        }
    }

    return true;
}

bool Grammar::Rule::operator!=(Rule const& rule) const {
    return !operator==(rule);
}

char const* IncorrectGrammarException::what() const throw() {
    return "Grammar is incorrect";
}

char const* GrammarOutOfSymbolsException::what() const throw() {
    return "Grammar exceeded symbol limit";
}

}
