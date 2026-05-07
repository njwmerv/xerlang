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

void print_stream_line(const std::vector<Token>& stream, std::ostream& err, const size_t i) {
    const size_t line_num = stream.at(i).line_num;
    size_t l = i;
    while (l >= 0 && stream.at(l).line_num == line_num) l--;
    l++;
    for (; stream.at(l).line_num == line_num; l++) {
        err << stream.at(l).lexeme << ' ';
    }
    err << '\n';
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

template <class T>
std::unique_ptr<T> unique_ptr_cast(SemanticValue& sv) {
    std::unique_ptr<T> ptr = std::unique_ptr<T>(dynamic_cast<T*>(sv.node.release()));
    if (!ptr) throw std::runtime_error{"ERROR: dynamic_cast failed"};
    return ptr;
}

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
                    for (i = 0; i < prod.len; i++) {
                        RHS.at(i) = std::move(stack.at(stack.size() - prod.len + i).data);
                    }
                    for (i = 0; i < prod.len; i++) {
                        stack.pop_back();
                    }

                    std::unique_ptr<ASTNode> new_node;
                    switch (pte.production_id) {
                        case start_BoFproceduresEoF: {
                            new_node = std::move(RHS.at(1).node);
                            break;
                        }
                        case procedures_dclSEMIprocedures: {
                            std::unique_ptr<ProgramNode> program = unique_ptr_cast<ProgramNode>(RHS.back());
                            program->global_vars.insert(program->global_vars.begin(), std::make_unique<VarInitNode>(unique_ptr_cast<DeclarationNode>(RHS.front())));
                            new_node = std::move(program);
                            break;
                        }
                        case procedures_dclBECOMESexpr1SEMIprocedures: {
                            std::unique_ptr<ProgramNode> program = unique_ptr_cast<ProgramNode>(RHS.back());
                            std::unique_ptr<VarInitNode> init = std::make_unique<VarInitNode>(unique_ptr_cast<DeclarationNode>(RHS.front()), unique_ptr_cast<ExprNode>(RHS.at(2)));
                            program->global_vars.insert(program->global_vars.begin(), std::move(init));
                            new_node = std::move(program);
                            break;
                        }
                        case procedures_structdefprocedures: {
                            std::unique_ptr<ProgramNode> program = unique_ptr_cast<ProgramNode>(RHS.back());
                            program->struct_defs.insert(program->struct_defs.begin(), unique_ptr_cast<StructDefNode>(RHS.front()));
                            new_node = std::move(program);
                            break;
                        }
                        case structdef_STRUCTIDLCURLYdclsRCURLYSEMI: {
                            new_node = std::make_unique<StructDefNode>(RHS.at(1).token.lexeme, unique_ptr_cast<DeclarationsNode>(RHS.at(3)));
                            break;
                        }
                        case procedures_procedureprocedures: {
                            std::unique_ptr<ProgramNode> program = unique_ptr_cast<ProgramNode>(RHS.back());
                            program->procedures.insert(program->procedures.begin(), unique_ptr_cast<ProcedureNode>(RHS.at(0)));
                            new_node = std::move(program);
                            break;
                        }
                        case procedure_IDCOLONLPARENparamsRPARENARROWtypeLCURLYstatementsRCURLY:
                        case procedure_IDCOLONLPARENparamsRPARENARROWVOIDLCURLYstatementsRCURLY: {
                            const std::string id = RHS.at(0).token.lexeme;
                            std::unique_ptr<DeclarationsNode> params = unique_ptr_cast<DeclarationsNode>(RHS.at(3));
                            std::unique_ptr<BlockNode> block = unique_ptr_cast<BlockNode>(RHS.at(8));
                            if (pte.production_id == procedure_IDCOLONLPARENparamsRPARENARROWVOIDLCURLYstatementsRCURLY) {
                                new_node = std::make_unique<ProcedureNode>(id, "void", std::move(params), std::move(block));
                            }
                            else {
                                std::unique_ptr<TypeNode> type = unique_ptr_cast<TypeNode>(RHS.at(6));
                                new_node = std::make_unique<ProcedureNode>(id, type->lexeme, std::move(params), std::move(block));
                            }
                            break;
                        }
                        case procedures_main: {
                            std::unique_ptr<ProgramNode> program = std::make_unique<ProgramNode>();
                            program->main = unique_ptr_cast<MainNode>(RHS.front());
                            new_node = std::move(program);
                            break;
                        }
                        case main_MAINCOLONLPARENRPARENARROWINTLCURLYstatementsRCURLY: {
                            new_node = std::make_unique<MainNode>(unique_ptr_cast<BlockNode>(RHS.at(7)));
                            break;
                        }
                        case statements_statementsstatement: {
                            // RHS[0] == Array, RHS[1] == next statement
                            std::unique_ptr<BlockNode> block = unique_ptr_cast<BlockNode>(RHS.front());
                            block->statements.push_back(unique_ptr_cast<StatementNode>(RHS.back()));
                            new_node = std::move(block);
                            break;
                        }
                        case args_expr1COMMAargs: {
                            // RHS[0] == arg, RHS[2] == arg_list
                            std::unique_ptr<ArgsNode> arg_list = unique_ptr_cast<ArgsNode>(RHS.back());
                            arg_list->args.insert(arg_list->args.begin(), unique_ptr_cast<ExprNode>(RHS.front()));
                            new_node = std::move(arg_list);
                            break;
                        }
                        case args_expr1: {
                            // end of arg_list
                            std::unique_ptr<ArgsNode> arg_list = std::make_unique<ArgsNode>();
                            arg_list->args.push_back(unique_ptr_cast<ExprNode>(RHS.front()));
                            new_node = std::move(arg_list);
                            break;
                        }
                        case paramlist_dclCOMMAparamlist:
                        case dcls_dclSEMIdcls: {
                            // RHS[0] == dcl, RHS[2] == dcl_list
                            std::unique_ptr<DeclarationsNode> dcl_list = unique_ptr_cast<DeclarationsNode>(RHS.back());
                            dcl_list->declarations.insert(dcl_list->declarations.begin(), unique_ptr_cast<DeclarationNode>(RHS.front()));
                            new_node = std::move(dcl_list);
                            break;
                        }
                        case dcl_typeID: {
                            new_node = std::make_unique<DeclarationNode>(unique_ptr_cast<TypeNode>(RHS.front())->lexeme, RHS.back().token.lexeme);
                            break;
                        }
                        case type_INTstar:
                        case type_CHARstar:
                        case type_BOOLstar:
                        case type_STRUCTIDstar: {
                            std::unique_ptr<StarNode> stars = unique_ptr_cast<StarNode>(RHS.back());
                            const std::string type = RHS.at(RHS.size() - 2).token.lexeme + std::string(stars->count, '*');
                            new_node = std::make_unique<TypeNode>(type);
                            break;
                        }
                        case star_ATstar: {
                            std::unique_ptr<StarNode> stars = unique_ptr_cast<StarNode>(RHS.back());
                            stars->count++;
                            new_node = std::move(stars);
                            break;
                        }
                        case star_: {
                            new_node = std::make_unique<StarNode>();
                            break;
                        }
                        case paramlist_dcl:
                        case dcls_dclSEMI: {
                            // end of dcl_list
                            std::unique_ptr<DeclarationsNode> dcl_list = std::make_unique<DeclarationsNode>();
                            dcl_list->declarations.push_back(unique_ptr_cast<DeclarationNode>(RHS.front()));
                            new_node = std::move(dcl_list);
                            break;
                        }
                        // Control Flow
                        case statement_dclBECOMESexpr1SEMI: {
                            new_node = std::make_unique<VarInitNode>(unique_ptr_cast<DeclarationNode>(RHS.front()), unique_ptr_cast<ExprNode>(RHS.at(2)));
                            break;
                        }
                        case statement_dclSEMI: {
                            new_node = std::move(unique_ptr_cast<DeclarationNode>(RHS.front()));
                            break;
                        }
                        case statement_expr1BECOMESexpr1SEMI: {
                            new_node = std::make_unique<AssignmentNode>(unique_ptr_cast<ExprNode>(RHS.front()), unique_ptr_cast<ExprNode>(RHS.at(2)));
                            break;
                        }
                        case statement_IFLPARENexpr1RPARENLCURLYstatementsRCURLYifs:
                        case ifs_ELIFLPARENexpr1RPARENLCURLYstatementsRCURLYifs: {
                            std::unique_ptr<IfNode> ifn = unique_ptr_cast<IfNode>(RHS.back());
                            ifn->clauses.insert(ifn->clauses.begin(), {unique_ptr_cast<ExprNode>(RHS.at(2)), unique_ptr_cast<BlockNode>(RHS.at(5))});
                            new_node = std::move(ifn);
                            break;
                        }
                        case ifs_: {
                            new_node = std::make_unique<IfNode>();
                            break;
                        }
                        case ifs_ELSELCURLYstatementsRCURLY: {
                            std::unique_ptr<IfNode> ifn = std::make_unique<IfNode>();
                            ifn->clauses.push_back({nullptr, unique_ptr_cast<BlockNode>(RHS.at(2))});
                            new_node = std::move(ifn);
                            break;
                        }
                        case statement_FORLPARENforprologueSEMIexpr1SEMIforepilogueRPARENLCURLYstatementsRCURLY: {
                            std::unique_ptr<ForPrologueNode> pro = unique_ptr_cast<ForPrologueNode>(RHS.at(2));
                            std::unique_ptr<ExprNode> cond = unique_ptr_cast<ExprNode>(RHS.at(4));
                            std::unique_ptr<StatementNode> epi = unique_ptr_cast<StatementNode>(RHS.at(6));
                            std::unique_ptr<BlockNode> block = unique_ptr_cast<BlockNode>(RHS.at(9));
                            new_node = std::make_unique<ForNode>(std::move(pro), std::move(cond), std::move(epi), std::move(block));
                            break;
                        }
                        case forprologue_dclBECOMESexpr1: {
                            std::unique_ptr<DeclarationNode> dcl = unique_ptr_cast<DeclarationNode>(RHS.front());
                            std::unique_ptr<ExprNode> expr = unique_ptr_cast<ExprNode>(RHS.back());
                            new_node = std::make_unique<ForPrologueNode>(std::make_unique<VarInitNode>(std::move(dcl), std::move(expr)));
                            break;
                        }
                        case forprologue_expr1BECOMESexpr1: {
                            std::unique_ptr<ExprNode> L = unique_ptr_cast<ExprNode>(RHS.front());
                            std::unique_ptr<ExprNode> R = unique_ptr_cast<ExprNode>(RHS.back());
                            new_node = std::make_unique<ForPrologueNode>(std::make_unique<AssignmentNode>(std::move(L), std::move(R)));
                            break;
                        }
                        case forepilogue_expr1BECOMESexpr1: {
                            std::unique_ptr<ExprNode> L = unique_ptr_cast<ExprNode>(RHS.front());
                            std::unique_ptr<ExprNode> R = unique_ptr_cast<ExprNode>(RHS.back());
                            new_node = std::make_unique<AssignmentNode>(std::move(L), std::move(R));
                            break;
                        }
                        case statement_BREAKSEMI: {
                            new_node = std::make_unique<BreakNode>();
                            break;
                        }
                        case statement_expr1SEMI: {
                            new_node = unique_ptr_cast<ExprNode>(RHS.at(0));
                            break;
                        }
                        case statement_DELETEexpr1SEMI: {
                            std::unique_ptr<ExprNode> ptr = unique_ptr_cast<ExprNode>(RHS.at(1));
                            new_node = std::make_unique<DeleteNode>(std::move(ptr));
                            break;
                        }
                        case statement_PRINTLPARENargsRPARENSEMI: {
                            new_node = std::make_unique<PrintNode>(unique_ptr_cast<ArgsNode>(RHS.at(2)));
                            break;
                        }
                        case statement_RETURNexpr1SEMI: {
                            new_node = std::make_unique<ReturnNode>(unique_ptr_cast<ExprNode>(RHS.at(1)));
                            break;
                        }
                        case statement_RETURNSEMI: {
                            new_node = std::make_unique<ReturnNode>(nullptr);
                            break;
                        }
                        case statement_WHILELPARENexpr1RPARENLCURLYstatementsRCURLY: {
                            new_node = std::make_unique<WhileNode>(unique_ptr_cast<ExprNode>(RHS.at(2)), unique_ptr_cast<BlockNode>(RHS.at(5)));
                            break;
                        }
                        case statements_: {
                            new_node = std::make_unique<BlockNode>();
                            break;
                        }
                        case params_: {
                            new_node = std::make_unique<DeclarationsNode>();
                            break;
                        }
                        case params_paramlist: {
                            new_node = std::move(RHS.front().node);
                            break;
                        }
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
                            new_node = std::make_unique<IDNode>(RHS.front().token.lexeme); break;
                        case expr14_TRUE: // expr14 -> TRUE
                            new_node = std::make_unique<TrueNode>(); break;
                        case expr14_FALSE: // expr14 -> FALSE
                            new_node = std::make_unique<FalseNode>(); break;
                        case expr14_NIL: // expr14 -> NIL
                            new_node = std::make_unique<NilNode>(); break;
                        case expr14_NUM: // expr14 -> NUM
                            new_node = std::make_unique<NumNode>(RHS.front().token.lexeme); break;
                        case expr14_CHARLIT: // expr14 -> CHARLIT
                            new_node = std::make_unique<CharNode>(RHS.front().token.lexeme); break;
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
                            new_node = std::make_unique<BinaryExprNode>(prod.RHS.at(1), unique_ptr_cast<ExprNode>(RHS.front()), unique_ptr_cast<ExprNode>(RHS.back()));
                            break;
                        }
                        case expr13_expr13ARROWID:
                        case expr13_expr13DOTID: { // arg->ID, arg.ID
                            new_node = std::make_unique<MemberAccessExprNode>(prod.RHS.at(1), unique_ptr_cast<ExprNode>(RHS.front()), RHS.back().token.lexeme);
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
                            new_node = std::make_unique<UnaryExprNode>(prod.RHS.front(), unique_ptr_cast<ExprNode>(RHS.back()));
                            break;
                        }
                        case expr13_expr13INCR:
                        case expr13_expr13DECR: { // arg op
                            new_node = std::make_unique<UnaryExprNode>(prod.RHS.back(), unique_ptr_cast<ExprNode>(RHS.front()));
                            break;
                        }
                        case expr14_READLPARENRPAREN: {
                            new_node = std::make_unique<ReadCallNode>();
                            break;
                        }
                        case expr14_IDLPARENargsRPAREN: {
                            new_node = std::make_unique<FunctionCallNode>(RHS.front().token.lexeme, unique_ptr_cast<ArgsNode>(RHS.at(2)));
                            break;
                        }
                        case expr14_IDLPARENRPAREN: {
                            new_node = std::make_unique<FunctionCallNode>(RHS.front().token.lexeme, nullptr);
                            break;
                        }
                        case expr14_NEWtypeLBRACKNUMRBRACK: {
                            std::unique_ptr<TypeNode> type = unique_ptr_cast<TypeNode>(RHS.at(1));
                            new_node = std::make_unique<AllocNode>(type->lexeme + '*', std::stoi(RHS.at(3).token.lexeme));
                            break;
                        }
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
        print_stream_line(stream, err, token_idx);
        err << "Parser Error in Line " << lookahead.line_num << " : Column " << lookahead.col_num << '\n';
        err << "Detected a Token w/ type: " << lookahead.type << " -> " << lookahead.lexeme << '\n';
        err << e.what() << std::endl;
    }
    throw std::runtime_error{"ERROR: Out - NO valid parse possible."};
    // theoretically, this ^^^ cannot occur, assuming correctness of LEXER implementation
}
