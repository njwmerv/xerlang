#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include "../util/types.h"

class Scanner{
    std::set<char> alphabet;
    std::set<ScannerDFAState> states;
    ScannerDFAState startState;
    std::map<std::string, ScannerDFAState> transitions;

    static Scanner* instance;

    Scanner();

    static void formatError(const std::string& message, uint32_t pos, uint32_t line);

    static bool isChar(const std::string& str);
    static bool isRange(const std::string& str);
    static bool isWhiteSpace(char c);

    static void formatToken(std::ostringstream& oss, ScannerDFAState& currentState, std::ofstream& tokens);

public:
    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(Scanner&&) = delete;

    static Scanner* getInstance();

    void scan(const std::string& fileNameStem) const;
};

#endif //SCANNER_H
