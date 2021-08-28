#include "nodes.h"

#include "module.h"

namespace ast {
// Top level declarations

void const_decl::build(modul & mod) const { mod.register_global(id, opt_type, *expr, true); }

void function_decl::build(modul & mod) const {
    mod.register_function(id, params, opt_ret_type, *body);
}

void struct_decl::build(modul & mod) const { mod.register_struct(id, fields); }

// Expressions

compiled_expr binary_expr::compile(modul &) const { return {}; }

compiled_expr if_expr::compile(modul &) const { return {}; }

compiled_expr literal::compile(modul & mod) const { return mod.compile_literal(value, typ); }

compiled_expr lvalue::compile(modul &) const { return {}; }

compiled_expr struct_init::compile(modul &) const { return {}; }

compiled_expr unary_expr::compile(modul &) const { return {}; }

// Statements

void assignment::build(modul &) const {}

void block_stmt::build(modul & mod) const {
    for (auto & stmt : stmts) stmt->build(mod);
}

void for_stmt::build(modul &) const {}

void function_call::build(modul & mod) const { mod.call_function(id, compile_args(mod)); }

compiled_expr function_call::compile(modul &) const { return {}; }

std::vector<compiled_expr> function_call::compile_args(modul & mod) const {
    std::vector<compiled_expr> compiled_args;
    compiled_args.reserve(args.size());
    for (auto & arg : args) compiled_args.push_back(arg->compile(mod));
    return compiled_args;
}

void if_stmt::build(modul &) const {}

void let_stmt::build(modul &) const {}

void return_stmt::build(modul &) const {}

void while_stmt::build(modul &) const {}

} // namespace ast
