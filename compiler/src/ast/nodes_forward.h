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

enum class binary_operation {
    add,
    sub,
    mul,
    div,
    rem,
    boolean_and,
    boolean_or,
    less_eq,
    less,
    greater_eq,
    greater,
    equal,
    not_equal,
    bit_and,
    bit_or,
    bit_xor,
    bit_left,
    bit_right
};

enum class unary_operation {
    boolean_not,
    negation,
    bit_not,
};

enum class assignment_operation {
    assign,
    add,
    sub,
    mul,
    div,
    remainder,
    bit_and,
    bit_or,
    bit_left,
    bit_right,
    bit_xor,
};

enum class type { string, integer, floating, character, boolean };
} // namespace ast
#endif
