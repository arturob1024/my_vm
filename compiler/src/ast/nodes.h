#ifndef NODES_H
#define NODES_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ast {

enum class node_type {};

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
class top_level : public virtual node {};
class statement : public virtual node {};
class expression : public virtual node {};

// The following two types are helper types.
// They have the following properites:
// - No polymorphism
// - No implicit copying
// - Read-only APIs
class typed_id final {
  public:
    typed_id(std::string * id, std::string * type)
        : id{std::move(*id)}
        , type{std::move(*type)} {
        delete id;
        delete type;
    }

    typed_id(const typed_id &) = delete;
    typed_id & operator=(const typed_id &) = delete;

    typed_id(typed_id &&) noexcept = default;
    typed_id & operator=(typed_id &&) noexcept = default;

    ~typed_id() noexcept = default;

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
    std::unique_ptr<expression> expr;
};

// declarations
class const_decl final : public top_level, public statement {
  public:
    const_decl(std::string * id, std::string * opt_type, expression * expr)
        : id{std::move(*id)}
        , expr{expr} {
        if (opt_type != nullptr) this->opt_type = std::move(*opt_type);
        delete id;
        delete opt_type;
    }

  private:
    std::string id;
    std::optional<std::string> opt_type;
    std::unique_ptr<expression> expr;
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

  private:
    std::string id;
    std::vector<typed_id> params;
    std::optional<std::string> opt_ret_type;
    std::unique_ptr<statement> body;
};
class struct_decl final : public top_level {
  public:
    struct_decl(std::string * id, std::vector<typed_id> && fields)
        : id{std::move(*id)}
        , fields{std::move(fields)} {
        delete id;
    }

  private:
    std::string id;
    std::vector<typed_id> fields;
};

// expressions
class binary_expr final : public expression {
  public:
    enum class operation {
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
    binary_expr(expression * lhs, operation op, expression * rhs)
        : lhs{lhs}
        , rhs{rhs}
        , op{op} {}

  private:
    std::unique_ptr<expression> lhs, rhs;
    operation op;
};
class if_expr final : public expression {
  public:
    if_expr(expression * cond, expression * true_case, expression * false_case)
        : cond{cond}
        , true_case{true_case}
        , false_case{false_case} {}

  private:
    std::unique_ptr<expression> cond, true_case, false_case;
};
class literal final : public expression {
  public:
    enum class type { string, integer, floating, character, boolean };
    literal(std::string * value, type typ)
        : value{std::move(*value)}
        , typ{typ} {
        delete value;
    }

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

  private:
    std::string id;
    std::unique_ptr<lvalue> parent;
};
class struct_init final : public expression {
  public:
    struct_init(std::string * type, std::vector<field_assignment> && fields)
        : type{std::move(*type)}
        , fields{std::move(fields)} {}

  private:
    std::string type;
    std::vector<field_assignment> fields;
};
class unary_expr final : public expression {
  public:
    enum class operation {
        boolean_not,
        negation,
        bit_not,
    };

    unary_expr(operation op, expression * expr)
        : op{op}
        , expr{expr} {}

  private:
    operation op;
    std::unique_ptr<expression> expr;
};

// statements
class assignment final : public statement {
  public:
    enum class operation {
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
    assignment(lvalue * dest, operation op, expression * expr)
        : dest{dest}
        , op{op}
        , expr{expr} {}

  private:
    std::unique_ptr<lvalue> dest;
    operation op;
    std::unique_ptr<expression> expr;
};
class block_stmt final : public statement {
  public:
    block_stmt() = default;
    explicit block_stmt(std::vector<statement *> && stmts) {
        for (auto * stmt : stmts) this->stmts.emplace_back(stmt);
    }

  private:
    std::vector<std::unique_ptr<statement>> stmts;
};
class for_stmt final : public statement {
  public:
    for_stmt(statement * initial, expression * condition, statement * increment, statement * body)
        : initial{initial}
        , increment{increment}
        , body{body}
        , condition{condition} {}

  private:
    std::unique_ptr<statement> initial, increment, body;
    std::unique_ptr<expression> condition;
};
class function_call final : public statement, public expression {
  public:
    function_call(std::string * id, std::vector<expression *> && args)
        : id{std::move(*id)} {
        for (auto * arg : args) this->args.emplace_back(arg);
        delete id;
    }

  private:
    std::string id;
    std::vector<std::unique_ptr<expression>> args;
};
class if_stmt final : public statement {
  public:
    if_stmt(expression * cond, statement * then_block, statement * else_block)
        : cond{cond}
        , then_block{then_block}
        , else_block{else_block} {}

  private:
    std::unique_ptr<expression> cond;
    std::unique_ptr<statement> then_block, else_block;
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

  private:
    std::string id;
    std::optional<std::string> opt_type;
    std::unique_ptr<expression> expr;
};
class return_stmt final : public statement {
  public:
    explicit return_stmt(expression * expr = nullptr)
        : expr{expr} {}

  private:
    std::unique_ptr<expression> expr;
};
class while_stmt final : public statement {
  public:
    while_stmt(expression * cond, statement * body)
        : cond{cond}
        , body{body} {}

  private:
    std::unique_ptr<expression> cond;
    std::unique_ptr<statement> body;
};

} // namespace ast

#endif
