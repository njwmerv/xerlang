#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>
#include <iostream>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>

class Scanner{
    std::set<char> alphabet;
    std::set<std::string> states;
    std::string startState;
    std::map<std::string, std::string> transitions;

    static Scanner* instance;

    Scanner();

    static void formatError(const std::string& message, uint32_t pos, uint32_t line);

    static bool isChar(const std::string& str);
    static bool isRange(const std::string& str);
    static bool isWhiteSpace(char c);

    static void formatToken(std::ostringstream& oss, std::string& currentState, std::ofstream& tokens);

public:
    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(Scanner&&) = delete;

    static Scanner* getInstance();

    void scan(const std::string& fileNameStem) const;
};

#endif //SCANNER_H
