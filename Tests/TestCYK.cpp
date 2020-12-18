#include <gtest/gtest.h>

#include <FL/ContextFreeGrammar.hpp>
#include <FL/CYK.hpp>
#include <vector>
#include <tuple>

using namespace FL;

struct CYKPrivate: public CYK {
    using CYK::CYK;
    using CYK::predict;
    using CYK::acceptsEmptyWord;
    using CYK::findDirectChildren;
    using CYK::initTable;
    using CYK::checkIfNonterminalGeneratesSubword;
    using CYK::calculateTableValues;
};

TEST(CYK, EmptyWord) {
    ContextFreeGrammar grammar({'a'}, {'A'}, 'A', {{"A", "a"}});
    CYK cyk(grammar);
    EXPECT_FALSE(cyk.predict(""));

    grammar = ContextFreeGrammar({'a'}, {'A', 'B'}, 'A', {{"A", "a"}, {"A", "B"}, {"B", ""}});
    cyk = CYK(grammar);
    EXPECT_TRUE(cyk.predict(""));
}

TEST(CYK, TableCreation) {
    ContextFreeGrammar grammar({'(', ')'}, {'S'}, 'S', {{"S", "SS"}, {"S", ""}, {"S", "(S)"}});
    CYKPrivate cyk(grammar);
    std::string word = "(())()()(((())()()))()((())()())()((()()))()()";
    auto table = cyk.calculateTableValues(word);

    auto shouldAcceptWord = [&](size_t subwordStart, size_t subwordEnd) {
        int balance = 0;
        for (size_t i = subwordStart; i <= subwordEnd; ++i) {
            if (word[i] == '(') {
                ++balance;
            } else {
                --balance;
            }
            if (balance < 0) {
                return false;
            }
        }
        return balance == 0;
    };

    for (size_t subwordStart = 0; subwordStart < word.size(); ++subwordStart) {
        for (size_t subwordEnd = subwordStart; subwordEnd < word.size(); ++subwordEnd) {
            EXPECT_EQ(
                table[cyk.grammar().startSymbol()][subwordStart][subwordEnd],
                shouldAcceptWord(subwordStart, subwordEnd)
            );
        }
    }

    EXPECT_TRUE(cyk.predict(word));
    word.pop_back();
    EXPECT_FALSE(cyk.predict(word));
}
