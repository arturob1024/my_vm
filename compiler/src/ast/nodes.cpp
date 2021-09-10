#include "nodes.h"

#include "ir/ir.h"

namespace ast {

using ir::operand;

// Top level declarations
module_and_file modul::open_module(const char * path) {
    return {std::make_unique<modul>(modul{path}), fopen(path, "r")};
}

module_and_file modul::open_stdin() { return {std::make_unique<modul>(modul{"stdin"}), stdin}; }

void modul::build(ir::modul & mod) const {
    for (auto & item : items) item->build(mod);
}

void modul::add_top_level_item(ast::top_level * top_lvl) { items.emplace_back(top_lvl); }

void const_decl::build(ir::modul & mod) const { mod.register_global(id, opt_type, *expr, true); }

void function_decl::build(ir::modul & mod) const {
    mod.register_function(id, params, opt_ret_type, *body);
}

void struct_decl::build(ir::modul & mod) const { mod.register_struct(id, fields); }

// Expressions

operand binary_expr::compile(ir::modul &) const { return {}; }

operand if_expr::compile(ir::modul &) const { return {}; }

operand literal::compile(ir::modul & mod) const { return mod.compile_literal(value, typ); }

operand lvalue::compile(ir::modul &) const { return {}; }

operand struct_init::compile(ir::modul &) const { return {}; }

operand unary_expr::compile(ir::modul &) const { return {}; }

// Statements

void assignment::build(ir::modul &) const {}

void block_stmt::build(ir::modul & mod) const {
    for (auto & stmt : stmts) stmt->build(mod);
}

void for_stmt::build(ir::modul &) const {}

void function_call::build(ir::modul & mod) const { mod.call_function(id, compile_args(mod)); }

operand function_call::compile(ir::modul &) const { return {}; }

std::vector<operand> function_call::compile_args(ir::modul & mod) const {
    std::vector<operand> compiled_args;
    compiled_args.reserve(args.size());
    for (auto & arg : args) compiled_args.push_back(arg->compile(mod));
    return compiled_args;
}

void if_stmt::build(ir::modul &) const {}

void let_stmt::build(ir::modul &) const {}

void return_stmt::build(ir::modul &) const {}

void while_stmt::build(ir::modul &) const {}

} // namespace ast
