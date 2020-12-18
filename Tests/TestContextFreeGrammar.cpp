#include <gtest/gtest.h>

#include <FL/ContextFreeGrammar.hpp>
#include <algorithm>
#include <vector>
#include <tuple>

using namespace FL;

struct ContextFreeGrammarPrivate: public ContextFreeGrammar {
    using ContextFreeGrammar::ContextFreeGrammar;
    using ContextFreeGrammar::removeLongRules;
    using ContextFreeGrammar::findEpsilonGenerators;
    using ContextFreeGrammar::removeEmptyRules;
    using ContextFreeGrammar::findChainedPairs;
    using ContextFreeGrammar::removeChainRules;
    using ContextFreeGrammar::findGeneratingNonterminals;
    using ContextFreeGrammar::removeNonGeneratingRules;
    using ContextFreeGrammar::findReachableNonterminals;
    using ContextFreeGrammar::removeNonReachableRules;
    using ContextFreeGrammar::removeMixedRules;
    using ContextFreeGrammar::isCorrect;
};

TEST(ContextFreeGrammar, Creation) {
    EXPECT_THROW(
        ContextFreeGrammar({'a'}, {'A'}, 'A', {{"A", "AA"}, {"aA", "a"}}),
        NonContextFreeGrammarException
    );
    EXPECT_THROW(
        ContextFreeGrammar(
            Grammar({'a'}, {'A'}, 'A', {{"A", "AA"}, {"AA", "a"}})
        ),
        NonContextFreeGrammarException
    );

    EXPECT_NO_THROW(ContextFreeGrammar(
        {'(', ')'},
        {'S'},
        'S',
        {{"S", "SS"}, {"S", ""}, {"S", "(S)"}}
    ));
}

TEST(ContextFreeGrammar, IsNormalized) {
    Alphabet terminals = {'a', 'b'};
    Alphabet nonterminals = {'S', 'A', 'B'};
    Symbol startSymbol = 'S';
    std::vector<std::tuple<std::vector<Grammar::Rule>, bool>> testCases = {
        {{{"S", "AB"}, {"A", "a"}, {"S", ""}, {"S", ""}}, true},
        {{{"S", "AB"}, {"A", "a"}, {"S", ""}, {"S", "S"}}, false},
        {{{"S", "AB"}, {"A", "a"}, {"S", ""}, {"S", "aA"}}, false},
        {{{"S", "AB"}, {"A", "a"}, {"S", ""}, {"A", "ab"}}, false},
        {{{"S", "AB"}, {"A", "a"}, {"S", ""}, {"A", ""}}, false},
        {{{"S", "AB"}, {"A", "a"}, {"S", ""}, {"S", "ASB"}}, false},
        {{{"S", "AB"}, {"A", "a"}, {"S", ""}, {"A", "S"}}, false}
    };

    for (auto const& [rules, isNormalized]: testCases) {
        EXPECT_EQ(
            ContextFreeGrammar(terminals, nonterminals, startSymbol, rules).isNormalized(),
            isNormalized
        );
    }
}

TEST(ContextFreeGrammar, RemoveLongRules) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B'},
        'S',
        {{"S", "AB"}, {"S", "ABS"}, {"S", "ABABABABABA"}, {"S", "S"}, {"S", "ABB"}}
    );
    grammar.removeLongRules();
    ASSERT_TRUE(grammar.isCorrect());

    for (auto const& rule: grammar.rules()) {
        EXPECT_EQ(rule.lhs.size(), 1);
        EXPECT_LE(rule.rhs.size(), 2);
    }
}

TEST(ContextFreeGrammar, FindEpsilonGenerators) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B', 'C'},
        'S',
        {{"S", "A"}, {"A", ""}, {"A", "BA"}, {"B", "A"}, {"S", "C"}, {"C", "ab"}}
    );
    auto epsilonGenerators = grammar.findEpsilonGenerators();
    EXPECT_EQ(epsilonGenerators.size(), 3);
    EXPECT_TRUE(epsilonGenerators.count('S'));
    EXPECT_TRUE(epsilonGenerators.count('A'));
    EXPECT_TRUE(epsilonGenerators.count('B'));

    grammar = ContextFreeGrammar({'a'}, {'A'}, 'A', {{"A", "AAA"}, {"A", "a"}});
    EXPECT_THROW(grammar.findEpsilonGenerators(), FoundLongRuleException);
}

TEST(ContextFreeGrammar, RemoveEmptyRules) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B', 'C'},
        'S',
        {{"S", "A"}, {"A", ""}, {"A", "BA"}, {"B", "A"}, {"S", "C"}, {"C", "ab"}, {"C", ""}}
    );
    grammar.removeEmptyRules();
    ASSERT_TRUE(grammar.isCorrect());

    for (auto const& rule: grammar.rules()) {
        EXPECT_EQ(rule.lhs.size(), 1);
        if (rule.lhs != Word{grammar.startSymbol()}) {
            EXPECT_NE(rule.rhs.size(), 0);
        }
    }

    grammar = ContextFreeGrammar({'a'}, {'A', 'B'}, 'A', {{"A", "AAA"}, {"A", ""}, {"B", "ABBA"}});
    EXPECT_THROW(grammar.removeEmptyRules(), FoundLongRuleException);
}

TEST(ContextFreeGrammar, FindChainedPairs) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B', 'C'},
        'S',
        {{"S", "A"}, {"A", "BA"}, {"B", "A"}, {"S", "C"}, {"C", "ab"}, {"C", "B"}}
    );
    auto chainedPairs = grammar.findChainedPairs();
    EXPECT_EQ(chainedPairs.size(), 6);
    std::vector<std::pair<Symbol, Symbol>> pairs = {
        {'S', 'A'}, {'B', 'A'}, {'S', 'A'}, {'S', 'C'}, {'S', 'B'}, {'C', 'B'}
    };
    for (auto const& pair: pairs) {
        EXPECT_TRUE(std::find(
            chainedPairs.begin(),
            chainedPairs.end(),
            pair
        ) != chainedPairs.end());
    }
}

TEST(ContextFreeGrammar, RemoveChainRules) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B', 'C'},
        'S',
        {{"S", "A"}, {"A", "B"}, {"B", "a"}, {"S", "C"}, {"C", "b"}}
    );
    grammar.removeChainRules();
    ASSERT_TRUE(grammar.isCorrect());

    for (auto const& [lhs, rhs]: grammar.rules()) {
        EXPECT_EQ(lhs.size(), 1);
        ASSERT_EQ(rhs.size(), 1);
        EXPECT_FALSE(grammar.symbolIsNonterminal(rhs[0]));
    }
}

TEST(ContextFreeGrammar, FindGeneratingNonterminals) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B', 'C', 'D'},
        'S',
        {{"S", "A"}, {"A", "B"}, {"B", "a"}, {"C", "b"}, {"C", "D"}}
    );
    auto generatingNonterminals = grammar.findGeneratingNonterminals();
    EXPECT_EQ(generatingNonterminals.size(), 4);
    EXPECT_TRUE(generatingNonterminals.count('S'));
    EXPECT_TRUE(generatingNonterminals.count('A'));
    EXPECT_TRUE(generatingNonterminals.count('B'));
    EXPECT_TRUE(generatingNonterminals.count('C'));

    grammar = ContextFreeGrammar({'a'}, {'A', 'B'}, 'A', {{"A", "AAA"}, {"A", ""}, {"B", "ABBA"}});
    EXPECT_THROW(grammar.findGeneratingNonterminals(), FoundLongRuleException);
}

TEST(ContextFreeGrammar, RemoveNonGeneratingRules) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B', 'C', 'D'},
        'S',
        {{"S", "A"}, {"A", "B"}, {"B", "a"}, {"C", "b"}, {"C", "D"}, {"D", "AD"}}
    );
    grammar.removeNonGeneratingRules();
    ASSERT_TRUE(grammar.isCorrect());

    for (auto const& [lhs, rhs]: grammar.rules()) {
        EXPECT_NE(lhs[0], 'D');
        EXPECT_EQ(std::find(rhs.begin(), rhs.end(), 'D'), rhs.end());
    }
}

TEST(ContextFreeGrammar, FindReachableNonterminals) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B', 'C', 'D', 'E', 'F'},
        'S',
        {{"S", "A"}, {"A", "BC"}, {"B", "a"}, {"C", "b"}, {"D", "CA"}, {"F", "DD"}, {"C", "EA"}}
    );
    auto reachableNonterminals = grammar.findReachableNonterminals();
    EXPECT_EQ(reachableNonterminals.size(), 5);
    EXPECT_TRUE(reachableNonterminals.count('S'));
    EXPECT_TRUE(reachableNonterminals.count('A'));
    EXPECT_TRUE(reachableNonterminals.count('B'));
    EXPECT_TRUE(reachableNonterminals.count('C'));
    EXPECT_TRUE(reachableNonterminals.count('E'));
}

TEST(ContextFreeGrammar, RemoveNonReachableRules) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B', 'C', 'D', 'E', 'F'},
        'S',
        {{"S", "A"}, {"A", "BC"}, {"B", "a"}, {"C", "b"}, {"D", "CA"}, {"F", "DD"}, {"C", "EA"}}
    );
    grammar.removeNonReachableRules();
    ASSERT_TRUE(grammar.isCorrect());

    for (auto const& [lhs, rhs]: grammar.rules()) {
        EXPECT_NE(lhs[0], 'D');
        EXPECT_EQ(std::find(rhs.begin(), rhs.end(), 'D'), rhs.end());
        EXPECT_NE(lhs[0], 'F');
        EXPECT_EQ(std::find(rhs.begin(), rhs.end(), 'F'), rhs.end());
    }
}

TEST(ContextFreeGrammar, RemoveMixedRules) {
    ContextFreeGrammarPrivate grammar(
        {'a', 'b'},
        {'S', 'A', 'B'},
        'S',
        {{"S", "A"}, {"S", "ab"}, {"S", "AB"}, {"A", "Aa"}, {"B", "bB"}}
    );
    grammar.removeMixedRules();
    ASSERT_TRUE(grammar.isCorrect());

    for (auto const& [lhs, rhs]: grammar.rules()) {
        if (rhs.size() != 2) {
            continue;
        }

        EXPECT_FALSE(grammar.symbolIsTerminal(rhs[0]));
        EXPECT_FALSE(grammar.symbolIsTerminal(rhs[1]));
    }

    grammar = ContextFreeGrammar({'a'}, {'A', 'B'}, 'A', {{"A", "aAa"}, {"A", ""}, {"B", "Aa"}});
    EXPECT_THROW(grammar.removeMixedRules(), FoundLongRuleException);
}

TEST(ContextFreeGrammar, Normalization) {
    ContextFreeGrammarPrivate grammar(
        {'(', ')'},
        {'S'},
        'S',
        {{"S", "SS"}, {"S", ""}, {"S", "(S)"}}
    );
    EXPECT_FALSE(grammar.isNormalized());
    ContextFreeGrammarPrivate normalized = grammar.normalized();
    ASSERT_TRUE(normalized.isCorrect());
    EXPECT_TRUE(normalized.isNormalized());
    EXPECT_TRUE(normalized.normalized().isNormalized());
}

TEST(ContextFreeGrammar, ExceptionMessages) {
    try {
        ContextFreeGrammar grammar(Grammar({'a'}, {'A'}, 'A', {{"A", "AA"}, {"aA", "a"}}));
    } catch (NonContextFreeGrammarException const& exception) {
        EXPECT_NO_THROW(exception.what());
    }

    try {
        ContextFreeGrammarPrivate grammar({'a'}, {'A'}, 'A', {{"A", "AAA"}, {"A", "a"}});
        grammar.findEpsilonGenerators();
    } catch (FoundLongRuleException const& exception) {
        EXPECT_NO_THROW(exception.what());
    }
}
