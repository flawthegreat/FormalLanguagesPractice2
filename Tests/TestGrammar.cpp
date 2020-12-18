#include <gtest/gtest.h>

#include <FL/Grammar.hpp>
#include <vector>
#include <tuple>
#include <limits>

using namespace FL;

struct GrammarPrivate: public Grammar {
    using Grammar::Grammar;
    using Grammar::addNewTerminal;
    using Grammar::addNewNonterminal;
};

TEST(Grammar, CreationAndCorrectness) {
    std::vector<std::tuple<
        Alphabet,
        Alphabet,
        Symbol,
        std::vector<Grammar::Rule>,
        bool
    >> testCases = {
        {{'a'}, {'A'}, 'A', {{"A", "a"}}, true},
        {{'b'}, {'A'}, 'A', {{"A", "a"}}, false},
        {{'a'}, {'B'}, 'A', {{"A", "a"}}, false},
        {{'a'}, {'A'}, 'B', {{"A", "a"}}, false},
        {{'a'}, {'A'}, 'A', {{"A", "b"}}, false},
        {{'a'}, {'A'}, 'A', {{"a", "a"}}, false},
        {{'a'}, {'A'}, 'A', {{"", "a"}}, false},
        {{1}, {2}, 2, {{Word{2}, Word{1}}}, true},
        {{1}, {2}, 2, {{Word{0}, Word{1}}}, false},
        {{'a', 'B'}, {'A', 'B'}, 'A', {{"A", "a"}}, false},
        {{'(', ')'}, {'S'}, 'S', {{"S", ""}, {"S", "SS"}, {"S", "(S)"}}, true},
    };

    for (auto const& [terminals, nonterminals, startSymbol, rules, isCorrect]: testCases) {
        if (isCorrect) {
            EXPECT_NO_THROW(Grammar(terminals, nonterminals, startSymbol, rules));
        } else {
            EXPECT_THROW(
                Grammar(terminals, nonterminals, startSymbol, rules),
                IncorrectGrammarException
            );
        }
    }

    GrammarPrivate grammar({}, {'S'}, 'S', {});
    EXPECT_NO_THROW(grammar.addNewTerminal());
    EXPECT_NO_THROW(grammar.addNewNonterminal());

    grammar = GrammarPrivate({Symbol{std::numeric_limits<int>::max()}}, {'S'}, 'S', {});
    EXPECT_THROW(grammar.addNewTerminal(), GrammarOutOfSymbolsException);
    EXPECT_THROW(grammar.addNewNonterminal(), GrammarOutOfSymbolsException);
}

TEST(Grammar, Getters) {
    Alphabet terminals = {'a', 'b', 'c'};
    Alphabet nonterminals = {'S', 'A', 'B', 'C'};
    Symbol startSymbol = 'S';
    std::vector<Grammar::Rule> rules = {{"S", "ABC"}, {"A", "a"}};
    Grammar grammar(terminals, nonterminals, startSymbol, rules);

    Alphabet grammarTerminals = grammar.terminals();
    Alphabet grammarNonterminals = grammar.nonterminals();
    Symbol grammarStartSymbol = grammar.startSymbol();
    std::vector<Grammar::Rule> grammarRules = grammar.rules();

    for (auto symbol: terminals) {
        EXPECT_TRUE(grammarTerminals.count(symbol));
    }
    for (auto symbol: grammarTerminals) {
        EXPECT_TRUE(terminals.count(symbol));
    }

    for (auto symbol: nonterminals) {
        EXPECT_TRUE(grammarNonterminals.count(symbol));
    }
    for (auto symbol: grammarNonterminals) {
        EXPECT_TRUE(nonterminals.count(symbol));
    }

    EXPECT_EQ(startSymbol, grammarStartSymbol);

    ASSERT_EQ(rules.size(), grammarRules.size());
    for (size_t i = 0; i < rules.size(); ++i) {
        EXPECT_EQ(rules[i], grammarRules[i]);
    }
}

TEST(Grammar, BasicProperties) {
    Alphabet terminals = {'a', 'b', 'c'};
    Alphabet nonterminals = {'S', 'A', 'B', 'C'};
    Symbol startSymbol = 'S';
    std::vector<Grammar::Rule> rules = {{"S", "ABC"}, {"A", "a"}};
    Grammar grammar(terminals, nonterminals, startSymbol, rules);

    EXPECT_TRUE(grammar.symbolIsTerminal('a'));
    EXPECT_FALSE(grammar.symbolIsTerminal('w'));
    EXPECT_TRUE(grammar.symbolIsNonterminal('C'));
    EXPECT_FALSE(grammar.symbolIsNonterminal('m'));
    EXPECT_TRUE(grammar.symbolIsCorrect('a'));
    EXPECT_TRUE(grammar.symbolIsCorrect('S'));
    EXPECT_FALSE(grammar.symbolIsCorrect(')'));
    EXPECT_TRUE(grammar.ruleIsCorrect({"S", "BAaBc"}));
    EXPECT_TRUE(grammar.ruleIsCorrect({"aBA", "cCc"}));
    EXPECT_FALSE(grammar.ruleIsCorrect({"aBAw", "cCc"}));
    EXPECT_FALSE(grammar.ruleIsCorrect({"aBA", "cCcw"}));
    EXPECT_FALSE(grammar.ruleIsCorrect({"w", "a"}));
    EXPECT_FALSE(grammar.ruleIsCorrect({"a", "a"}));
    EXPECT_TRUE(grammar.ruleIsCorrect({"A", ""}));
}

TEST(Grammar, BasicRuleComparison) {
    EXPECT_EQ(Grammar::Rule("A", "abacka"), Grammar::Rule("A", "abacka"));
    EXPECT_NE(Grammar::Rule("AB", "abacka"), Grammar::Rule("A", "abacka"));
    EXPECT_NE(Grammar::Rule("A", "abacka"), Grammar::Rule("A", "abawka"));
    EXPECT_NE(Grammar::Rule("A", "abacka"), Grammar::Rule("A", "abawkala"));
    EXPECT_NE(Grammar::Rule("ROL", "abacka"), Grammar::Rule("RIL", "abacka"));
}

TEST(Grammar, BasicContextFree) {
    Alphabet terminals = {'a', 'b', 'c'};
    Alphabet nonterminals = {'S', 'A', 'B', 'C'};
    Symbol startSymbol = 'S';
    std::vector<Grammar::Rule> rules = {{"SA", "ABC"}, {"A", "a"}};
    Grammar grammar(terminals, nonterminals, startSymbol, rules);
    EXPECT_FALSE(grammar.isContextFree());

    rules = {{"S", "ABABABABABACS"}, {"C", "C"}, {"A", "a"}};
    grammar = Grammar(terminals, nonterminals, startSymbol, rules);
    EXPECT_TRUE(grammar.isContextFree());
}

TEST(Grammar, ExceptionMessages) {
    try {
        Grammar grammar({}, {}, 'S', {});
    } catch (IncorrectGrammarException const& exception) {
        EXPECT_NO_THROW(exception.what());
    }

    try {
        GrammarPrivate grammar({Symbol{std::numeric_limits<int>::max()}}, {'S'}, 'S', {});
        grammar.addNewTerminal();
    } catch (GrammarOutOfSymbolsException const& exception) {
        EXPECT_NO_THROW(exception.what());
    }
}
