#include "parser.h"
#include <iostream>
#include <vector>
#include <memory>
#include "parser_constants.h"
#include "ast.h"

using namespace Parser;

std::ostream& operator<<(std::ostream& os, Parser::ParserSymbol state) {
    switch (state) {
        case MAIN: os << "MAIN"; break;
        case READ: os << "READ"; break;
        case PRINT: os << "PRINT"; break;
        case INT: os << "INT"; break;
        case CHAR: os << "CHAR"; break;
        case BOOL: os << "BOOL"; break;
        case STRUCT: os << "STRUCT"; break;
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
        case BoF: os << "BoF"; break;
        case EoF: os << "EoF"; break;
        case start: os << "start"; break;
        case procedures: os << "procedures"; break;
        case procedure: os << "procedure"; break;
        case main: os << "main"; break;
        case type: os << "type"; break;
        case star: os << "star"; break;
        case params: os << "params"; break;
        case dcls: os << "dcls"; break;
        case dcl: os << "dcl"; break;
        case statements: os << "statements"; break;
        case statement: os << "statement"; break;
        case ifs: os << "ifs"; break;
        case forprologue: os << "forprologue"; break;
        case forepilogue: os << "forepilogue"; break;
        case expr1: os << "expr1"; break;
        case expr2: os << "expr2"; break;
        case expr3: os << "expr3"; break;
        case expr4: os << "expr4"; break;
        case expr5: os << "expr5"; break;
        case expr6: os << "expr6"; break;
        case expr7: os << "expr7"; break;
        case expr8: os << "expr8"; break;
        case expr9: os << "expr9"; break;
        case expr10: os << "expr10"; break;
        case expr11: os << "expr11"; break;
        case expr12: os << "expr12"; break;
        case expr13: os << "expr13"; break;
        case expr14: os << "expr14"; break;
        case args: os << "args"; break;
        case structdef: os << "structdef"; break;
        default: os << "NONE"; break;
    }
    return os;
}

// Constructing Concrete Syntax Tree (CST)

struct SemanticValue {
    Token token;
    std::unique_ptr<ASTNode> node = nullptr;
};

struct LALRData {
    uint16_t state = 0;
    SemanticValue data;
};

std::vector<LALRData> stack;

std::unique_ptr<ASTNode> parse(const std::vector<Token>& stream, std::ostream& err) {
    if (!stack.empty()) stack.clear();
    stack.push_back({0, {{}, nullptr}});
    size_t token_idx = 0;

    try {
        while (token_idx < stream.size()) {
            const Token& lookahead = stream.at(token_idx);
            uint16_t current_state = stack.back().state;

            const ParsingTableEntry& pte = PARSING_TABLE[current_state][lookahead.type];

            switch (pte.act) {
                case ParsingTableEntry::Action::SHIFT:
                    stack.push_back({pte.target_state, {lookahead, {}}});
                    token_idx++;
                    break;
                case ParsingTableEntry::Action::REDUCE: {
                    const Production& prod = PRODUCTIONS[pte.production_id];

                    std::vector<SemanticValue> RHS(prod.len);
                    int i;
                    for (i = 0; i < prod.len; i++) RHS.at(prod.len - 1 - i) = std::move(stack.at(stack.size() - prod.len + i).data);
                    for (i = 0; i < prod.len; i++) stack.pop_back();

                    std::unique_ptr<ASTNode> new_node;
                    for (SemanticValue& data : RHS) {
                        if (data.node) data.node->parent = new_node.get();
                    }
                    switch (pte.production_id) {
                        case expr1_expr2:
                        case expr2_expr3:
                        case expr3_expr4:
                        case expr4_expr5:
                        case expr5_expr6:
                        case expr6_expr7:
                        case expr7_expr8:
                        case expr8_expr9:
                        case expr9_expr10:
                        case expr10_expr11:
                        case expr11_expr12:
                        case expr12_expr13:
                        case expr13_expr14:
                        case forepilogue_expr1:
                            // exprX -> expr(X+1), a -> b
                            new_node = std::move(RHS.at(0).node); break;
                        case expr14_LPARENexpr1RPAREN: // expr14 -> ( expr1 )
                            new_node = std::move(RHS.at(1).node); break;
                        case expr14_ID: // expr14 -> ID
                            new_node = std::make_unique<IDNode>(lookahead.lexeme); break;
                        case expr14_TRUE: // expr14 -> TRUE
                            new_node = std::make_unique<TrueNode>(); break;
                        case expr14_FALSE: // expr14 -> FALSE
                            new_node = std::make_unique<FalseNode>(); break;
                        case expr14_NIL: // expr14 -> NIL
                            new_node = std::make_unique<NilNode>(); break;
                        case expr14_NUM: // expr14 -> NUM
                            new_node = std::make_unique<NumNode>(lookahead.lexeme); break;
                        case expr14_CHARLIT: // expr14 -> CHARLIT
                            new_node = std::make_unique<CharNode>(lookahead.lexeme); break;
                        case expr1_expr1ORexpr2:
                        case expr2_expr2ANDexpr3:
                        case expr3_expr3BITORexpr4:
                        case expr4_expr4BITXORexpr5:
                        case expr5_expr5BITANDexpr6:
                        case expr6_expr6EQUALSexpr7:
                        case expr6_expr6NEQexpr7:
                        case expr7_expr7LTexpr8:
                        case expr7_expr7LEQexpr8:
                        case expr7_expr7GTexpr8:
                        case expr7_expr7GEQexpr8:
                        case expr8_expr8LSHIFTexpr9:
                        case expr8_expr8RSHIFTexpr9:
                        case expr9_expr9PLUSexpr10:
                        case expr9_expr9SUBexpr10:
                        case expr10_expr10MULTexpr11:
                        case expr10_expr10DIVexpr11:
                        case expr10_expr10MODexpr11:
                        case expr12_expr13EXPexpr12: {
                            // arg1 op arg2
                            std::unique_ptr<ExprNode> arg1 = std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(0).node.release()));
                            std::unique_ptr<ExprNode> arg2 = std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(2).node.release()));
                            new_node = std::make_unique<BinaryExprNode>(prod.RHS.at(1), std::move(arg1), std::move(arg2));
                            break;
                        }
                        case expr13_expr13ARROWID:
                        case expr13_expr13DOTID: { // arg->ID, arg.ID
                            std::unique_ptr<ExprNode> arg = std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(0).node.release()));
                            std::unique_ptr<IDNode> ID = std::make_unique<IDNode>(RHS.at(2).token.lexeme);
                            new_node = std::make_unique<MemberAccessExprNode>(prod.RHS.at(1), std::move(arg), std::move(ID));
                            break;
                        }
                        case expr11_ATexpr11:
                        case expr11_ADDRexpr11:
                        case expr11_NOTexpr11:
                        case expr11_BITNOTexpr11:
                        case expr11_INCRexpr11:
                        case expr11_DECRexpr11:
                        case expr11_SUBexpr11:
                        case expr11_PLUSexpr11: { // op arg
                            std::unique_ptr<ExprNode> arg = std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(1).node.release()));
                            new_node = std::make_unique<UnaryExprNode>(prod.RHS.at(0), std::move(arg));
                            break;
                        }
                        case expr13_expr13INCR:
                        case expr13_expr13DECR: { // arg op
                            std::unique_ptr<ExprNode> arg = std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(0).node.release()));
                            new_node = std::make_unique<UnaryExprNode>(prod.RHS.at(1), std::move(arg));
                            break;
                        }
                        case statements_statementsstatement: {
                            // RHS[0] == Array, RHS[1] == next statement
                            std::unique_ptr<BlockNode> block = std::unique_ptr<BlockNode>(dynamic_cast<BlockNode*>(RHS.at(0).node.release()));
                            std::unique_ptr<StatementNode> statement = std::unique_ptr<StatementNode>(dynamic_cast<StatementNode*>(RHS.at(1).node.release()));
                            block->statements.push_back(std::move(statement));
                            new_node = std::move(block);
                            break;
                        }
                        case args_expr1COMMAargs: {
                            // RHS[0] == arg, RHS[2] == arg_list
                            std::unique_ptr<ArgsNode> arg_list = std::unique_ptr<ArgsNode>(dynamic_cast<ArgsNode*>(RHS.at(2).node.release()));
                            std::unique_ptr<ExprNode> arg = std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(0).node.release()));
                            arg_list->args.insert(arg_list->args.begin(), std::move(arg));
                            new_node = std::move(arg_list);
                            break;
                        }
                        case args_expr1: {
                            // end of arg_list
                            std::unique_ptr<ArgsNode> arg_list = std::make_unique<ArgsNode>();
                            arg_list->args.push_back(std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(0).node.release())));
                            new_node = std::move(arg_list);
                            break;
                        }
                        case paramlist_dclCOMMAparamlist:
                        case dcls_dclSEMIdcls: {
                            // RHS[0] == dcl, RHS[2] == dcl_list
                            std::unique_ptr<DeclarationsNode> dcl_list = std::unique_ptr<DeclarationsNode>(dynamic_cast<DeclarationsNode*>(RHS.at(2).node.release()));
                            std::unique_ptr<DeclarationNode> dcl = std::unique_ptr<DeclarationNode>(dynamic_cast<DeclarationNode*>(RHS.at(0).node.release()));
                            dcl_list->declarations.insert(dcl_list->declarations.begin(), std::move(dcl));
                            new_node = std::move(dcl_list);
                            break;
                        }
                        case paramlist_dcl:
                        case dcls_dclSEMI: {
                            // end of dcl_list
                            std::unique_ptr<DeclarationsNode> dcl_list = std::make_unique<DeclarationsNode>();
                            dcl_list->declarations.push_back(std::unique_ptr<DeclarationNode>(dynamic_cast<DeclarationNode*>(RHS.at(0).node.release())));
                            new_node = std::move(dcl_list);
                            break;
                        }
                        // Control Flow
                        case statement_dclBECOMESexpr1SEMI: {
                            std::unique_ptr<DeclarationNode> dcl = std::unique_ptr<DeclarationNode>(dynamic_cast<DeclarationNode*>(RHS.at(0).node.release()));
                            std::unique_ptr<ExprNode> val = std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(2).node.release()));
                            new_node = std::make_unique<VarInitNode>(std::move(dcl), std::move(val));
                            break;
                        }
                        case statement_dclSEMI: {
                            break;
                        }
                        case statement_expr1BECOMESexpr1SEMI: {
                            break;
                        }
                        case statement_IFLPARENexpr1RPARENLCURLYstatementsRCURLYifs: {
                            break;
                        }
                        case statement_WHILELPARENexpr1RPARENLCURLYstatementsRCURLY: {
                            std::unique_ptr<ExprNode> condition = std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(RHS.at(2).node.release()));
                            std::unique_ptr<BlockNode> block = std::unique_ptr<BlockNode>(dynamic_cast<BlockNode*>(RHS.at(5).node.release()));
                            new_node = std::make_unique<WhileNode>(std::move(condition), std::move(block));
                            break;
                        }
                        case statements_:
                            new_node = std::make_unique<BlockNode>();
                            break;
                        case params_:
                            new_node = std::make_unique<DeclarationsNode>();
                            break;
                        case params_paramlist:
                            new_node = std::move(RHS.at(0).node);
                            break;
                        default:
                            throw std::runtime_error{"ERROR: Unknown production used to produce AST node"};
                            // ^^^ this shouldn't be possible, theoretically...
                    }

                    if(stack.empty()) throw std::runtime_error{"ERROR: Attempting to read empty stack!"};
                    // theoretically, this ^^^ cannot occur, assuming correctness of parser implementation
                    uint16_t state_after_pop = stack.back().state;

                    const ParsingTableEntry& goto_pte = PARSING_TABLE[state_after_pop][prod.LHS];
                    if(goto_pte.act != ParsingTableEntry::Action::GOTO) throw std::runtime_error{"ERROR: NOT GOTO entry found!"};
                    // theoretically, this ^^^ cannot occur, assuming correctness of parser implementation

                    stack.push_back({goto_pte.target_state, {{}, std::move(new_node)}});
                    break;
                }
                case ParsingTableEntry::Action::ACCEPT:
                    return std::move(stack.back().data.node);
                default:
                    throw std::runtime_error{"ERROR: Default - NO valid parse possible."};
            }
        }
    } catch (std::exception& e) {
        const Token& lookahead = stream.at(token_idx);
        err << "Parser Error in Line " << lookahead.line_num << " : Column " << lookahead.col_num << '\n';
        err << "Detected a Token w/ type: " << lookahead.type << " -> " << lookahead.lexeme << '\n';
        err << e.what() << std::endl;
    }
    throw std::runtime_error{"ERROR: Out - NO valid parse possible."};
    // theoretically, this ^^^ cannot occur, assuming correctness of LEXER implementation
}

// Construct Abstract Syntax Tree (AST)



// Combined Steps
