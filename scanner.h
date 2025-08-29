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
    std::unordered_map<std::string, bool> states;
    std::string startState;
    std::map<std::string, std::string> transitions;

    static Scanner* instance;

    Scanner();

    static void formatError(const std::string& message);

    static bool isChar(const std::string& str);
    static bool isRange(const std::string& str);
    static bool isWhiteSpace(const char c);

    static void formatToken(const std::ostringstream& oss, std::string& currentState);

public:
    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    Scanner(Scanner&&) = delete;
    Scanner& operator=(Scanner&&) = delete;

    static Scanner* getInstance();

    void scan(const std::string& fileNameStem) const;
};

#endif //SCANNER_H
