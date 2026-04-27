#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <unordered_map>

// Scanner
typedef std::string ScannerDFAState;
typedef std::string Token;

// Parser
typedef int ParserDFAState;
typedef std::string Symbol; // (non-)terminal

const std::unordered_map<Symbol, int> tokenToInt = {
};

#endif //TYPES_H
