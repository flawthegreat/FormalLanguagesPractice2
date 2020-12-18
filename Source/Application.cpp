#include <iostream>
#include <FL/ContextFreeGrammar.hpp>
#include <FL/CYK.hpp>

int main() {
    using namespace FL;

    char inputSeparator = '^';
    std::cout << "(Use " << inputSeparator << " to end input)" << std::endl;
    std::cout << "Terminals: ";
    Alphabet terminals;
    char symbol;
    while (std::cin >> symbol && symbol != inputSeparator) {
        terminals.insert(symbol);
    }

    std::cout << "Nonterminals: ";
    Alphabet nonterminals;
    while (std::cin >> symbol && symbol != inputSeparator) {
        nonterminals.insert(symbol);
    }

    std::cout << "Start symbol: ";
    std::cin >> symbol;
    Symbol startSymbol = symbol;

    std::cout << "Rules (lhs[space]rhs|<eps>): ";
    std::string lhs;
    std::string rhs;
    std::vector<Grammar::Rule> rules;
    while (std::cin >> lhs && lhs != std::string(1, inputSeparator)) {
        std::cin >> rhs;
        if (rhs == "<eps>") {
            rhs.clear();
        }
        rules.emplace_back(lhs, rhs);
    }

    std::string word;
    try {
        ContextFreeGrammar grammar(terminals, nonterminals, startSymbol, rules);
        CYK cyk(grammar);

        for (;;) {
            std::cout << "Word to test: ";
            std::cin >> word;
            if (word == std::string(1, inputSeparator)) {
                break;
            }
            std::cout << (cyk.predict(word) ? "accept" : "reject") << std::endl;
        }
    } catch(std::exception const& exception) {
        std::cout << exception.what() << std::endl;
    }
}
