#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unordered_map>

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
        start, BoF, EoF,
        procedures, procedure, main,
        type, star,
        params, dcls, dcl,
        statements, statement, ifs, forprologue, forepilogue,
        lvalue, expr1, expr2, expr3, expr4, expr5, expr6, expr7, expr8, expr9,
        args, structdef,

        // MISC
        ERROR, NUM_SYMBOLS,
    };
}

typedef int Type;

struct Production;

struct ASTNode;

struct Procedure;

#endif //PARSER_H
