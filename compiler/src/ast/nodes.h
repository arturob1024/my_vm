#ifndef NODES_H
#define NODES_H

#include <string>
#include <vector>

namespace ast {

enum class node_type {};

class node {};

// Intermediate nodes
class top_level : public virtual node {};
class statement : public virtual node {};
class expression : public virtual node {};

// declarations
class const_decl final : public top_level, public statement {
  public:
    const_decl(std::string *, std::string *, expression *) {}
};
class function_decl final : public top_level {};
class struct_decl final : public top_level {};

// expressions
class binary_expr final : public expression {};
class if_expr final : public expression {
  public:
    if_expr([[maybe_unused]] expression * cond, [[maybe_unused]] expression * true_case,
            [[maybe_unused]] expression * false_case) {}
};
class literal final : public expression {};
class lvalue final : public expression {};
class struct_init final : public expression {};
class unary_expr final : public expression {};

// statements
class assignment final : public statement {};
class block_stmt final : public statement {
  public:
    block_stmt() = default;
    explicit block_stmt(std::vector<statement *> &&) {}
};
class for_stmt final : public statement {
  public:
    for_stmt([[maybe_unused]] statement * initial, [[maybe_unused]] expression * condition,
             [[maybe_unused]] statement * increment, [[maybe_unused]] statement * body) {}
};
class function_call final : public statement, public expression {
  public:
    function_call(std::string *, std::vector<expression *> &&) {}
};
class if_stmt final : public statement {
  public:
    if_stmt([[maybe_unused]] expression * cond, [[maybe_unused]] statement * then_block,
            [[maybe_unused]] statement * else_block) {}
};
class let_stmt final : public statement {
  public:
    let_stmt(std::string *, std::string *, expression *) {}
};
class return_stmt final : public statement {
  public:
    explicit return_stmt([[maybe_unused]] expression * expr = nullptr) {}
};
class while_stmt final : public statement {
  public:
    while_stmt([[maybe_unused]] expression * cond, [[maybe_unused]] statement * body) {}
};

} // namespace ast

#endif
