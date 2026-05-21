#ifndef VISITOR_HELPER_H
#define VISITOR_HELPER_H

#include <memory>
#include "../util/types.h"

inline bool is_pointer(const std::string& type);

bool is_lvalue(const std::unique_ptr<ExprNode>& node);

inline bool is_struct(const std::string& type);

inline bool is_struct_pointer(const std::string& type);

inline bool is_proc_node(Parser::ParserSymbol ps);

void get_parent_nodes(ASTNode* start, ASTNode** prog = nullptr, ASTNode** proc = nullptr);

#endif // VISITOR_HELPER_H
