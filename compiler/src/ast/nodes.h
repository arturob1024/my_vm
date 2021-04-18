#ifndef NODES_H
#define NODES_H

namespace ast {
enum class node_type {};

class node {};

// Intermediate nodes
class top_level : public virtual node {};
class statement : public virtual node {};
class expression : public virtual node {};

// declarations
class const_decl final : public top_level, public statement {};
class function_decl final : public top_level {};
class struct_decl final : public top_level {};

// expressions
class binary_expr final : public expression {};
class if_expr final : public expression {};
class literal final : public expression {};
class lvalue final : public expression {};
class struct_init final : public expression {};
class unary_expr final : public expression {};

// statements
class assignment final : public statement {};
class block_stmt final : public statement {};
class for_stmt final : public statement {};
class function_call final : public statement, public expression {};
class if_stmt final : public statement {};
class let_stmt final : public statement {};
class return_stmt final : public statement {};
class while_stmt final : public statement {};

} // namespace ast

#endif
