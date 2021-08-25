#ifndef NODES_FORWARD_H
#define NODES_FORWARD_H

#include <memory>
namespace ast {

enum class node_type {};

class node;

class top_level;
using top_level_ptr = std::unique_ptr<top_level>;
class statement;
using statement_ptr = std::unique_ptr<statement>;
class expression;
using expression_ptr = std::unique_ptr<expression>;

class typed_id;
class field_assignment;
class const_decl;
class function_decl;
class struct_decl;
class binary_expr;
class if_expr;
class literal;
class lvalue;
class struct_init;
class unary_expr;
class assignment;
class block_stmt;
class for_stmt;
class function_call;
class if_stmt;
class let_stmt;
class return_stmt;
class while_stmt;

} // namespace ast
#endif
