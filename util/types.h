#ifndef TYPES_H
#define TYPES_H

#include <memory>

#define MAX_RHS_LEN 11

namespace Scanner {
    enum ScannerDFAState {
        // ACCEPTINGs
        MAIN, READ, PRINT, INT, CHAR, BOOL, STRUCT,
        TRUE, FALSE, NIL, NUM, CHARLIT, ID,
        RETURN, IF, ELIF, ELSE, FOR, WHILE, BREAK,
        DELETE, NEW,
        COLON, LPAREN, RPAREN, SEMI, LCURLY, RCURLY, COMMA, LBRACK, RBRACK, BECOMES,
        NOT, OR, AND, GEQ, GT, LEQ, LT, EQUALS, NEQ,
        PLUS, SUB, MULT, DIV, MOD, LSHIFT, RSHIFT, EXP,
        BITOR, BITXOR, BITAND, BITNOT,
        AT, ADDR, INCR, DECR, ARROW, DOT,

        // NON-ACCEPTINGs
        START, APOS, APOSLASH, NOT_CHARLIT,
        ERROR, NUM_STATES,
    };
}

std::ostream& operator<<(std::ostream& os, Scanner::ScannerDFAState state);

namespace Parser {
    enum ParserSymbol {
        // TERMINATING
        MAIN, READ, PRINT, INT, CHAR, BOOL, STRUCT,
        TRUE, FALSE, NIL, NUM, CHARLIT, ID,
        RETURN, IF, ELIF, ELSE, FOR, WHILE, BREAK,
        DELETE, NEW,
        COLON, LPAREN, RPAREN, SEMI, LCURLY, RCURLY, COMMA, LBRACK, RBRACK, BECOMES,
        NOT, OR, AND, GEQ, GT, LEQ, LT, EQUALS, NEQ,
        PLUS, SUB, MULT, DIV, MOD, LSHIFT, RSHIFT, EXP,
        BITOR, BITXOR, BITAND, BITNOT,
        AT, ADDR, INCR, DECR, ARROW, DOT,

        // NON-TERMINATING
        BoF, EoF, start,
        procedures, procedure, main,
        type, star,
        params, dcls, dcl,
        statements, statement, ifs, forprologue, forepilogue,
        expr1, expr2, expr3, expr4, expr5, expr6, expr7, expr8, expr9, expr10, expr11, expr12, expr13, expr14,
        args, structdef,
        NUM_SYMBOLS,
    };
}

std::ostream& operator<<(std::ostream& os, Parser::ParserSymbol state);

typedef struct {
    std::string lexeme;
    Parser::ParserSymbol type;
} Token;

typedef int Type;

struct Production {
    const Parser::ParserSymbol LHS;
    const size_t len;
    const std::array<Parser::ParserSymbol, MAX_RHS_LEN> RHS;
};

struct ASTNode {
    ASTNode* parent;
    Production production;
    Type type;
    Parser::ParserSymbol node_type;
    std::string lexeme;
    std::vector<std::unique_ptr<ASTNode*>> children;

    ~ASTNode() { parent = nullptr; }
};

#endif //TYPES_H
