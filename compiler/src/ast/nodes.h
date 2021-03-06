#ifndef NODES_H
#define NODES_H

#include "ir/ir_forward.h"
#include "nodes_forward.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ast {

class node {
  public:
    node() noexcept = default;

    node(const node &) = delete;
    node & operator=(const node &) = delete;

    node(node &&) noexcept = default;
    node & operator=(node &&) noexcept = default;

    virtual ~node() noexcept = default;
};

// Intermediate nodes
class top_level : public virtual node {
  public:
    virtual void build(ir::modul &) const = 0;
};

class statement : public virtual node {
  public:
    virtual void build(ir::modul &) const = 0;
};

class expression : public virtual node {
  public:
    virtual ir::operand compile(ir::modul &) const = 0;
};

// The following two types are helper types.
// They have the following properites:
// - No polymorphism
// - Read-only APIs
// Implicit copy will be permitted on a per-type basis, as some types may benefit from copying.
class typed_id final {
  public:
    typed_id(std::string * id, std::string * type)
        : id{std::move(*id)}
        , type{std::move(*type)} {
        delete id;
        delete type;
    }

    [[nodiscard]] std::pair<std::string, std::string> id_and_type() const { return {id, type}; }

  private:
    std::string id;
    std::string type;
};

class field_assignment final {
  public:
    field_assignment(std::string * id, expression * expr)
        : id{std::move(*id)}
        , expr{expr} {
        delete id;
    }

    field_assignment(const field_assignment &) = delete;
    field_assignment & operator=(const field_assignment &) = delete;

    field_assignment(field_assignment &&) noexcept = default;
    field_assignment & operator=(field_assignment &&) noexcept = default;

    ~field_assignment() noexcept = default;

  private:
    std::string id;
    expression_ptr expr;
};

// declarations
class modul final : public top_level {
  public:
    static module_and_file open_module(const char * path);
    static module_and_file open_stdin();

    void build(ir::modul &) const final;

    void add_top_level_item(top_level *);
    [[nodiscard]] size_t top_level_item_count() const noexcept { return items.size(); }

    std::string filename() const { return file_name; }

  private:
    explicit modul(std::string filename)
        : file_name{std::move(filename)} {}

    std::vector<top_level_ptr> items;
    std::string file_name;
};

class const_decl final : public top_level, public statement {
  public:
    const_decl(std::string * id, std::string * opt_type, expression * expr)
        : id{std::move(*id)}
        , expr{expr} {
        if (opt_type != nullptr) this->opt_type = std::move(*opt_type);
        delete id;
        delete opt_type;
    }

    void build(ir::modul & mod) const final;

  private:
    std::string id;
    std::optional<std::string> opt_type;
    expression_ptr expr;
};
class function_decl final : public top_level {
  public:
    function_decl(std::string * id, std::vector<typed_id> && params, std::string * opt_ret_type,
                  statement * body)
        : id{std::move(*id)}
        , params{std::move(params)}
        , body{body} {
        if (opt_ret_type != nullptr) this->opt_ret_type = std::move(*opt_ret_type);
        delete id;
        delete opt_ret_type;
    }

    void build(ir::modul & mod) const final;

  private:
    std::string id;
    std::vector<typed_id> params;
    std::optional<std::string> opt_ret_type;
    statement_ptr body;
};
class struct_decl final : public top_level {
  public:
    struct_decl(std::string * id, std::vector<typed_id> && fields)
        : id{std::move(*id)}
        , fields{std::move(fields)} {
        delete id;
    }

    void build(ir::modul & mod) const final;

  private:
    std::string id;
    std::vector<typed_id> fields;
};

// expressions
class binary_expr final : public expression {
  public:
    binary_expr(expression * lhs, binary_operation op, expression * rhs)
        : lhs{lhs}
        , rhs{rhs}
        , op{op} {}

    ir::operand compile(ir::modul &) const final;

  private:
    expression_ptr lhs, rhs;
    binary_operation op;
};
class if_expr final : public expression {
  public:
    if_expr(expression * cond, expression * true_case, expression * false_case)
        : cond{cond}
        , true_case{true_case}
        , false_case{false_case} {}

    ir::operand compile(ir::modul &) const final;

  private:
    expression_ptr cond, true_case, false_case;
};
class literal final : public expression {
  public:
    literal(std::string * value, type typ)
        : value{std::move(*value)}
        , typ{typ} {
        delete value;
    }

    ir::operand compile(ir::modul & mod) const final;

  private:
    std::string value;
    type typ;
};
class lvalue final : public expression {
  public:
    explicit lvalue(std::string * id, lvalue * parent = nullptr)
        : id{std::move(*id)}
        , parent{parent} {
        delete id;
    }

    ir::operand compile(ir::modul &) const final;

  private:
    std::string id;
    std::unique_ptr<lvalue> parent;
};
class struct_init final : public expression {
  public:
    struct_init(std::string * type, std::vector<field_assignment> && fields)
        : type{std::move(*type)}
        , fields{std::move(fields)} {}

    ir::operand compile(ir::modul &) const final;

  private:
    std::string type;
    std::vector<field_assignment> fields;
};
class unary_expr final : public expression {
  public:
    unary_expr(unary_operation op, expression * expr)
        : op{op}
        , expr{expr} {}

    ir::operand compile(ir::modul &) const final;

  private:
    unary_operation op;
    expression_ptr expr;
};

// statements
class assignment final : public statement {
  public:
    assignment(lvalue * dest, assignment_operation op, expression * expr)
        : dest{dest}
        , op{op}
        , expr{expr} {}

    void build(ir::modul &) const final;

  private:
    std::unique_ptr<lvalue> dest;
    assignment_operation op;
    expression_ptr expr;
};
class block_stmt final : public statement {
  public:
    block_stmt() = default;
    explicit block_stmt(std::vector<statement *> && stmts) {
        for (auto * stmt : stmts) this->stmts.emplace_back(stmt);
    }

    void build(ir::modul & mod) const final;

  private:
    std::vector<statement_ptr> stmts;
};
class for_stmt final : public statement {
  public:
    for_stmt(statement * initial, expression * condition, statement * increment, statement * body)
        : initial{initial}
        , increment{increment}
        , body{body}
        , condition{condition} {}

    void build(ir::modul &) const final;

  private:
    statement_ptr initial, increment, body;
    expression_ptr condition;
};
class function_call final : public statement, public expression {
  public:
    function_call(std::string * id, std::vector<expression *> && args)
        : id{std::move(*id)} {
        for (auto * arg : args) this->args.emplace_back(arg);
        delete id;
    }

    void build(ir::modul & mod) const final;

    ir::operand compile(ir::modul &) const final;

  private:
    std::vector<ir::operand> compile_args(ir::modul & mod) const;

    std::string id;
    std::vector<expression_ptr> args;
};
class if_stmt final : public statement {
  public:
    if_stmt(expression * cond, statement * then_block, statement * else_block)
        : cond{cond}
        , then_block{then_block}
        , else_block{else_block} {}

    void build(ir::modul &) const final;

  private:
    expression_ptr cond;
    statement_ptr then_block, else_block;
};
class let_stmt final : public statement {
  public:
    let_stmt(std::string * id, std::string * opt_type, expression * expr)
        : id{std::move(*id)}
        , expr{expr} {
        if (opt_type != nullptr) this->opt_type = std::move(*opt_type);
        delete id;
        delete opt_type;
    }

    void build(ir::modul &) const final;

  private:
    std::string id;
    std::optional<std::string> opt_type;
    expression_ptr expr;
};
class return_stmt final : public statement {
  public:
    explicit return_stmt(expression * expr = nullptr)
        : expr{expr} {}

    void build(ir::modul &) const final;

  private:
    expression_ptr expr;
};
class while_stmt final : public statement {
  public:
    while_stmt(expression * cond, statement * body)
        : cond{cond}
        , body{body} {}

    void build(ir::modul &) const final;

  private:
    expression_ptr cond;
    statement_ptr body;
};

} // namespace ast

#endif
