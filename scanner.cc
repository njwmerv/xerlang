#include "scanner.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

const std::string rawAlphabet = R"(a-z A-Z 0-9 ( ) { } [ _ ] = ! < > + - * / | ^ ~ % , : ; ' " &)";
const std::string rawStates =
R"(s
MAIN! INT! CHAR! BOOL! AUTO! CONST! RETURN! IF! ELSIF! ELSE!
FOR! WHILE! DO! BREAK! DELETE! GOTO! NEW! SIZEOF! TRUE! FALSE!
ID! DEC! HEX! BIN! STRLIT! CHARLIT! 0! NULL!
notHEX notBIN notSTR char0 char1 charS
:! (! )! -! ;! {! }! ,! [! ]! =!
||! &&! ?! >=! >! <=! <! ==! !=!
+! -! *! /! %! **! <<! >>! ^! |! &!
!! ~! ++! --! ->! .!
)";
const std::string rawTransitions =
R"(s a-z A-Z _ ID
    ID a-z A-Z _ 0-9 ID
s , ,
s ; ;
s : :
s ( (
s ) )
s { {
s } }
s [ [
s ] ]
s + +
    + + ++
s - -
    - - --
    - > ->
    - 1-9 DEC
s * *
    * * **
s / /
s % %
s < <
    < < <<
    < = <=
s > >
    > > >>
    > = >=
s ^ ^
s | |
    | | ||
s & &
    & & &&
s ! !
    ! = !=
s ~ ~
s . .
s = =
    = = ==
s ? ?
s 0 0
    0 x notHEX
    0 b notBIN
notHEX 0-9 a-f A-F HEX
    HEX 0-9 a-f A-F HEX
notBIN 0-1 BIN
    BIN 0-1 BIN
s 1-9 DEC
    DEC 0-9 DEC
s " notSTR
    notSTR !-~ notSTR
    notSTR " STRLIT
s ' char0
    char0 !-[ char1
    char0 ]-~ char1
    char0 \ charS
    charS n t b r a ' " ? \ f v 0 char1
    char1 ' CHARLIT
)";

Scanner* Scanner::instance = nullptr;

Scanner::Scanner(){
    // Create alphabet
    std::istringstream iss{rawAlphabet};
    std::string str;
    while(iss >> str){
        if(isChar(str)) alphabet.insert(str.front());
        else{
            for(char c = str.front(); c <= str.back(); c++) alphabet.insert(c);
        }
    }
    alphabet.insert(' ');
    alphabet.insert('\t');
    alphabet.insert('\v');

    // Create DFA States
    bool readFirst = false;
    iss.clear();
    iss.str(rawStates);
    while(iss >> str){
        bool accepting = false;
        if(str.back() == '!' && !isChar(str)){
            str.pop_back();
            accepting = true;
        }
        states.insert({str, accepting});

        if(!readFirst){
            startState = str;
            readFirst = true;
        }
    }

    // Create DFA Transitions
    iss.clear();
    iss.str(rawTransitions);
    while(std::getline(iss, str)){
        std::string fromState, symbols, toState;
        std::istringstream line{str};
        std::vector<std::string> lineVector;
        while(line >> str) lineVector.push_back(str);
        fromState = lineVector.front();
        toState = lineVector.back();
        lineVector.pop_back();
        for(int i = 1; i < lineVector.size(); i++) {
            std::string& s = lineVector[i];
            if(isChar(s)) symbols += s;
            else if(isRange(s)){
                for(char c = s.front(); c <= s.back(); c++) symbols += c;
            }
        }
        for(const char c : symbols) transitions.insert({fromState + c, toState});
    }
    transitions.insert({"char0 ", "char1"});
    transitions.insert({"notSTR ", "notSTR"});
    transitions.insert({"notSTR\t", "notSTR"});
    transitions.insert({"notSTR\v", "notSTR"});

    // for(const auto& [from, to] : transitions){
    //     std::cout << from.substr(0, from.size() - 1) << " == " << from.back() << " => " << to << std::endl;
    // }
}

Scanner* Scanner::getInstance(){
    if(instance == nullptr) instance = new Scanner();
    return instance;
}

bool Scanner::isChar(const std::string& str) {return str.length() == 1; }
bool Scanner::isRange(const std::string& str) {return str.length() == 3;}
bool Scanner::isWhiteSpace(const char c) {return c == ' ' || c == '\t' || c == '\v';}
void Scanner::formatError(const std::string& message) {std::cerr << "ERROR: " << message << std::endl;}
void Scanner::formatToken(const std::ostringstream& oss, std::string& currentState){
    if(currentState == "0") currentState = "DEC";
    else if(oss.str() == "main" && currentState == "ID") currentState = "MAIN";
    else if(oss.str() == "int" && currentState == "ID") currentState = "INT";
    else if(oss.str() == "char" && currentState == "ID") currentState = "CHAR";
    else if(oss.str() == "bool" && currentState == "ID") currentState = "BOOL";
    else if(oss.str() == "auto" && currentState == "ID") currentState = "auto";
    else if(oss.str() == "const" && currentState == "ID") currentState = "CONST";
    else if(oss.str() == "return" && currentState == "ID") currentState = "RETURN";
    else if(oss.str() == "if" && currentState == "ID") currentState = "IF";
    else if(oss.str() == "elsif" && currentState == "ID") currentState = "ELSIF";
    else if(oss.str() == "else" && currentState == "ID") currentState = "ELSE";
    else if(oss.str() == "for" && currentState == "ID") currentState = "FOR";
    else if(oss.str() == "while" && currentState == "ID") currentState = "WHILE";
    else if(oss.str() == "do" && currentState == "ID") currentState = "DO";
    else if(oss.str() == "break" && currentState == "ID") currentState = "BREAK";
    else if(oss.str() == "delete" && currentState == "ID") currentState = "DELETE";
    else if(oss.str() == "goto" && currentState == "ID") currentState = "GOTO";
    else if(oss.str() == "new" && currentState == "ID") currentState = "NEW";
    else if(oss.str() == "sizeof" && currentState == "ID") currentState = "SIZEOF";
    else if(oss.str() == "true" && currentState == "ID") currentState = "TRUE";
    else if(oss.str() == "false" && currentState == "ID") currentState = "FALSE";
    else if(oss.str() == "NULL" && currentState == "ID") currentState = "NULL";
}

void Scanner::scan(const std::string& fileNameStem) const {
    std::ifstream file{fileNameStem + ".xer"};
    std::ofstream tokens{fileNameStem + ".tokens"};
    tokens.clear();

    bool error = false;
    std::string str;
    while(std::getline(file, str) && !error){
        std::string currentState = startState;

        // Ignore anything after "//", skip entirely if there's nothing before
        const size_t commentStart = str.find("//");
        str = str.substr(0, commentStart);
        if(str.empty()) continue;

        // Tokenize the line
        std::istringstream lineSS{str};
        std::ostringstream lexeme;
        char c;
        while(lineSS.get(c) && !error){
            try{
                if(isWhiteSpace(c) && currentState == startState) continue;
                if(isWhiteSpace(c) && (currentState != "char0" || currentState != "notSTR")) throw std::invalid_argument{"got whitespace."};
                if(!alphabet.contains(c)) throw std::invalid_argument{"Invalid character encountered: " + c};
                const std::string& newState = transitions.at(currentState + c);

                if(currentState == "DEC" && (std::stoll(lexeme.str() + c) > INT_MAX || std::stoll(lexeme.str() + c) < INT_MIN)){
                    throw std::invalid_argument{"Given out-of-range dec, starting new num."};
                }
                if(currentState == "BIN" && std::stoll(lexeme.str().substr(2, lexeme.str().size()) + c, nullptr, 2) > INT_MAX){
                    throw std::invalid_argument{"Given out-of-range bin, starting new num."};
                }
                if(currentState == "HEX" && std::stoll(lexeme.str().substr(2, lexeme.str().size()) + c, nullptr, 16) > INT_MAX){
                    throw std::invalid_argument{"Given out-of-range hex, starting new num."};
                }
                currentState = newState;
                lexeme << c;
            }
            catch(std::exception& e){
                if(states.at(currentState)){
                    formatToken(lexeme, currentState);

                    tokens << currentState << " : " << lexeme.str() << "\n";

                    lexeme.str("");
                    lexeme.clear();
                    currentState = startState;
                    if(transitions.contains(startState + c)){
                        currentState = transitions.at(startState + c);
                        lexeme << c;
                    }
                    else if(!isWhiteSpace(c)){
                        error = true;
                        formatError("Invalid start of token found: " + lexeme.str());
                        break;
                    }
                }
                else{
                    error = true;
                    formatError(e.what());
                    break;
                }
            }
        }
        // END OF LINE
        if(states.at(currentState)){
            formatToken(lexeme, currentState);

            tokens << currentState << " : " << lexeme.str() << "\n";

            lexeme.str("");
            lexeme.clear();
            currentState = startState;
            if(transitions.contains(startState + c)){
                currentState = transitions.at(startState + c);
                lexeme << c;
            }
            else if(!isWhiteSpace(c)){
                error = true;
                formatError("Invalid start of token found: " + lexeme.str());
                break;
            }
        }
        else if(!isWhiteSpace(c)){
            error = true;
            break;
        }
    }
}
