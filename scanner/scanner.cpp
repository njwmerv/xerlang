#include "scanner.h"
#include <array>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <limits>

using namespace Scanner;

inline bool is_accepting_state(Scanner::ScannerDFAState state) {
    return state != START && state != APOS && state != APOSLASH && state != NOT_CHARLIT;
}

std::ostream& operator<<(std::ostream& os, Scanner::ScannerDFAState state) {
    switch (state) {
        case MAIN: os << "MAIN"; break;
        case READ: os << "READ"; break;
        case PRINT: os << "PRINT"; break;
        case INT: os << "INT"; break;
        case CHAR: os << "CHAR"; break;
        case BOOL: os << "BOOL"; break;
        case TRUE: os << "TRUE"; break;
        case FALSE: os << "FALSE"; break;
        case NIL: os << "NIL"; break;
        case NUM: os << "NUM"; break;
        case CHARLIT: os << "CHARLIT"; break;
        case ID: os << "ID"; break;
        case RETURN: os << "RETURN"; break;
        case IF: os << "IF"; break;
        case ELIF: os << "ELIF"; break;
        case ELSE: os << "ELSE"; break;
        case FOR: os << "FOR"; break;
        case WHILE: os << "WHILE"; break;
        case BREAK: os << "BREAK"; break;
        case DELETE: os << "DELETE"; break;
        case NEW: os << "NEW"; break;
        case COLON: os << "COLON"; break;
        case LPAREN: os << "LPAREN"; break;
        case RPAREN: os << "RPAREN"; break;
        case SEMI: os << "SEMI"; break;
        case LCURLY: os << "LCURLY"; break;
        case RCURLY: os << "RCURLY"; break;
        case COMMA: os << "COMMA"; break;
        case LBRACK: os << "LBRACK"; break;
        case RBRACK: os << "RBRACK"; break;
        case BECOMES: os << "BECOMES"; break;
        case NOT: os << "NOT"; break;
        case OR: os << "OR"; break;
        case AND: os << "AND"; break;
        case GEQ: os << "GEQ"; break;
        case GT: os << "GT"; break;
        case LEQ: os << "LEQ"; break;
        case LT: os << "LT"; break;
        case EQUALS: os << "EQUALS"; break;
        case NEQ: os << "NEQ"; break;
        case PLUS: os << "PLUS"; break;
        case SUB: os << "SUB"; break;
        case MULT: os << "MULT"; break;
        case AT: os << "AT"; break;
        case ADDR: os << "ADDR"; break;
        case DIV: os << "DIV"; break;
        case MOD: os << "MOD"; break;
        case LSHIFT: os << "LSHIFT"; break;
        case RSHIFT: os << "RSHIFT"; break;
        case EXP: os << "EXP"; break;
        case BITOR: os << "BITOR"; break;
        case BITXOR: os << "BITXOR"; break;
        case BITAND: os << "BITAND"; break;
        case BITNOT: os << "BITNOT"; break;
        case INCR: os << "INCR"; break;
        case DECR: os << "DECR"; break;
        case ARROW: os << "ARROW"; break;
        case DOT: os << "DOT"; break;
        case STRUCT: os << "STRUCT"; break;
        case START: os << "START"; break;
        case APOS: os << "APOS"; break;
        case NOT_CHARLIT: os << "NOT_CHARLIT"; break;
        default: os << "NONE"; break;
    }
    return os;
}

using Transitions = std::array<std::array<ScannerDFAState, 128>, NUM_STATES>;
consteval Transitions buildTransitions() {
    Transitions t{};

    for (auto& row : t) row.fill(ERROR);

    char c;
    for (c = 'a'; c <= 'z'; c++) {
        t[START][c] = ID;
        t[ID][c] = ID;
    }
    for (c = 'A'; c <= 'Z'; c++) {
        t[START][c] = ID;
        t[ID][c] = ID;
    }
    t[START]['_'] = t[ID]['_'] = ID;

    for (c = '0'; c <= '9'; c++) {
        t[START][c] = NUM;
        t[ID][c] = ID;
        t[NUM][c] = NUM;
    }

    t[START][':'] = COLON;
    t[START]['('] = LPAREN;
    t[START][')'] = RPAREN;
    t[START][';'] = SEMI;
    t[START]['{'] = LCURLY;
    t[START]['}'] = RCURLY;
    t[START][','] = COMMA;
    t[START]['['] = LBRACK;
    t[START][']'] = RBRACK;

    t[START]['='] = BECOMES;
    t[BECOMES]['='] = EQUALS;

    t[START]['!'] = NOT;
    t[NOT]['='] = NEQ;

    t[START]['~'] = BITNOT;

    t[START]['|'] = BITOR;
    t[BITOR]['|'] = OR;

    t[START]['&'] = BITAND;
    t[BITAND]['&'] = AND;

    t[START]['>'] = GT;
    t[GT]['='] = GEQ;
    t[GT]['>'] = RSHIFT;

    t[START]['<'] = LT;
    t[LT]['='] = LEQ;
    t[LT]['<'] = LSHIFT;

    t[START]['+'] = PLUS;
    t[PLUS]['+'] = INCR;

    t[START]['*'] = MULT;

    t[START]['@'] = AT;

    t[START]['$'] = ADDR;

    t[START]['/'] = DIV;

    t[START]['%'] = MOD;

    t[START]['^'] = BITXOR;
    t[BITXOR]['^'] = EXP;

    t[START]['.'] = DOT;

    t[START]['-'] = SUB;
    t[SUB]['-'] = DECR;
    t[SUB]['>'] = ARROW;

    t[NOT_CHARLIT]['\''] = CHARLIT;

    t[START]['\''] = APOS;

    for (c = ' '; c <= '~'; c++) t[APOS][c] = NOT_CHARLIT;

    t[APOSLASH]['\\'] = NOT_CHARLIT;
    t[APOSLASH]['\"'] = NOT_CHARLIT;
    t[APOSLASH]['\''] = NOT_CHARLIT;
    t[APOSLASH]['\?'] = NOT_CHARLIT;
    t[APOS]['\\'] = APOSLASH;
    t[APOS]['\"'] = ERROR;
    t[APOS]['\''] = ERROR;
    t[APOS]['\?'] = ERROR;
    t[APOSLASH]['n'] = NOT_CHARLIT;
    t[APOSLASH]['t'] = NOT_CHARLIT;
    t[APOSLASH]['r'] = NOT_CHARLIT;
    t[APOSLASH]['b'] = NOT_CHARLIT;
    t[APOSLASH]['a'] = NOT_CHARLIT;
    t[APOSLASH]['0'] = NOT_CHARLIT;
    t[APOSLASH]['f'] = NOT_CHARLIT;
    t[APOSLASH]['v'] = NOT_CHARLIT;

    return t;
}

constexpr Transitions transitions = buildTransitions();

const std::unordered_map<std::string_view, ScannerDFAState> KEYWORDS = {
    {"main", MAIN}, {"read", READ}, {"print", PRINT}, {"int", INT}, {"char", CHAR}, {"bool", BOOL}, {"struct", STRUCT},
    {"true", TRUE}, {"false", FALSE}, {"NULL", NIL},
    {"return", RETURN}, {"if", IF}, {"elif", ELIF}, {"else", ELSE}, {"for", FOR}, {"while", WHILE}, {"break", BREAK},
    {"delete", DELETE}, {"new", NEW},
};

void catch_num_error(const std::string& lexeme) {
    const long long val = std::strtoll(lexeme.c_str(), nullptr, 10);
    if (val > INT32_MAX || val < INT32_MIN) throw std::exception{};
    if (lexeme.at(0) == '0' && lexeme.length() > 1) throw std::exception{};
}

void scan(std::istream& is, std::ostream& os, std::vector<Token>& stream, std::ostream& err = std::cerr) {
    ScannerDFAState state = START;
    std::string lexeme;

    char c;
    size_t line_num = 1;
    try{
        while (is.get(c)) {
            if (c == '\n') line_num++;
            if (c == '#') { // COMMENT -> skip line!
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                line_num++;
            }
            else if (transitions[state][c] == ERROR && state == START) { // INVALID TOKEN START!
                if(c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\r') continue;
                throw std::exception{};
            }
            else if (transitions[state][c] == ERROR) { // NEW TOKEN!
                is.putback(c);

                if (state == ID && KEYWORDS.contains(lexeme)) state = KEYWORDS.at(lexeme);
                else if (state == NUM) catch_num_error(lexeme);

                os << state  << " : " << lexeme << std::endl;
                stream.push_back(Token{std::move(lexeme), state});

                lexeme.clear();
                state = START;
            }
            else if (transitions[state][c] != ERROR) { // REGULAR TRANSITION!
                lexeme += c;
                state = transitions[state][c];
            }
        }
    } catch (std::exception& e) {
        is.putback(c);
        std::string line;
        std::getline(is, line, '\r');
        err << lexeme << line << std::endl;
        for (int i = 0; i < lexeme.length()-1; i++) err << ' ';
        err << "^~~~ ERROR in line#" << line_num << std::endl;
        return;
    }
    if (!lexeme.empty() && is_accepting_state(state)) { // Leftovers!
        if (state == ID && KEYWORDS.contains(lexeme)) state = KEYWORDS.at(lexeme);
        else if (state == NUM) catch_num_error(lexeme);

        os << state  << " : " << lexeme << std::endl;
        stream.push_back(Token{std::move(lexeme), state});
    }
}
