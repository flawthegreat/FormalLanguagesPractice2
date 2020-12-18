#include "Common.hpp"

namespace FL {

Symbol::Symbol(int rawValue):
    rawValue(rawValue)
{}

Symbol::Symbol(char character):
    rawValue(character)
{}

bool Symbol::operator==(Symbol const& symbol) const {
    return rawValue == symbol.rawValue;
}

bool Symbol::operator!=(Symbol const& symbol) const {
    return !operator==(symbol);
}

}

namespace std {

size_t hash<FL::Symbol>::operator()(FL::Symbol const& symbol) const {
    return hash<decltype(symbol.rawValue)>()(symbol.rawValue);
}

}
