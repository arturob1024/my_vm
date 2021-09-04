#include "ir.h"

#include "ast/nodes.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

namespace ir {

type_ptr string_type::instance = std::make_shared<string_type>();
type_ptr integer_type::instance = std::make_shared<integer_type>();
type_ptr floating_type::instance = std::make_shared<floating_type>();
type_ptr boolean_type::instance = std::make_shared<boolean_type>();
type_ptr character_type::instance = std::make_shared<character_type>();
type_ptr unit_type::instance = std::make_shared<unit_type>();

type_ptr ast_to_ir_type(const std::string & ast) {

    if (ast == "string") {
        return string_type::instance;
    } else if (ast.empty()) {
        return unit_type::instance;
    }
    std::cout << "Unimplemented ir type for '" << ast << '\'' << std::endl;
    assert(false);
}

void string_type::print(std::ostream & lhs) const { lhs << "string"; }
void integer_type::print(std::ostream & lhs) const { lhs << "integer"; }
void floating_type::print(std::ostream & lhs) const { lhs << "floating"; }
void character_type::print(std::ostream & lhs) const { lhs << "character"; }
void boolean_type::print(std::ostream & lhs) const { lhs << "boolean"; }
void unit_type::print(std::ostream & lhs) const { lhs << "unit"; }
void func_type::print(std::ostream & lhs) const {
    lhs << '(';
    for (auto & typ : arg_types) lhs << *typ << ", ";
    lhs << ") " << *ret_type;
}

// Top level item compilation

void modul::register_global(std::string, std::optional<std::string>, ast::expression &, bool) {}
void modul::register_function(std::string id, const std::vector<ast::typed_id> & params,
                              const std::optional<std::string> & type, ast::statement & body) {

    assert(functions.find(id) == functions.end());

    std::vector<id_and_type> parameters;
    for (auto & param : params) {
        auto [id, type] = param.id_and_type();
        parameters.push_back(std::make_pair(id, ast_to_ir_type(type)));
    }
    functions.insert_or_assign(
        id, function_details{{}, std::move(parameters), type.value_or(""), func_num++});
    current_func_name = id;
    body.build(*this);
    current_func_name.clear();
}

void modul::register_struct(std::string, const std::vector<ast::typed_id> &) {}

// Statment compilation

void modul::call_function(std::string id, std::vector<operand> args) {

    auto iter = functions.find(id);
    assert(iter != functions.end());

    args.insert(args.begin(), {id, iter->second.func_type()});
    current_function().instructions.emplace_back(operation::call, args, std::nullopt);
}

// Expression compilation

operand modul::compile_literal(const std::string & value, ast::type typ) {
    switch (typ) {
    case ast::type::string:
        return {value, string_type::instance};
    case ast::type::integer:
        return {value, integer_type::instance};
    case ast::type::floating:
        return {value, floating_type::instance};
    case ast::type::character:
        return {value, character_type::instance};
    case ast::type::boolean:
        return {value, boolean_type::instance};
    default:
        std::cout << "Unsupported ast type: " << (int)typ << std::endl;
        exit(2);
    }
}

operand modul::compile_binary_op(ast::binary_operation op, operand lhs, operand rhs) {
    ir::operation ir_op;
    switch (op) {
    case ast::binary_operation::add:
        ir_op = ir::operation::add;
        break;
    case ast::binary_operation::sub:
        ir_op = ir::operation::sub;
        break;
    case ast::binary_operation::mul:
        ir_op = ir::operation::mul;
        break;
    case ast::binary_operation::div:
        ir_op = ir::operation::div;
        break;
    case ast::binary_operation::rem:
        ir_op = ir::operation::rem;
        break;
    case ast::binary_operation::boolean_and:
        ir_op = ir::operation::boolean_and;
        break;
    case ast::binary_operation::boolean_or:
        ir_op = ir::operation::boolean_or;
        break;
    case ast::binary_operation::less_eq:
        ir_op = ir::operation::less_eq;
        break;
    case ast::binary_operation::less:
        ir_op = ir::operation::less;
        break;
    case ast::binary_operation::greater_eq:
        ir_op = ir::operation::greater_eq;
        break;
    case ast::binary_operation::greater:
        ir_op = ir::operation::greater;
        break;
    case ast::binary_operation::equal:
        ir_op = ir::operation::equal;
        break;
    case ast::binary_operation::not_equal:
        ir_op = ir::operation::not_equal;
        break;
    case ast::binary_operation::bit_and:
        ir_op = ir::operation::bit_and;
        break;
    case ast::binary_operation::bit_or:
        ir_op = ir::operation::bit_or;
        break;
    case ast::binary_operation::bit_xor:
        ir_op = ir::operation::bit_xor;
        break;
    case ast::binary_operation::bit_left:
        ir_op = ir::operation::bit_left;
        break;
    case ast::binary_operation::bit_right:
        ir_op = ir::operation::bit_right;
        break;
    default:
        std::cout << "Unsupported ast binary op: " << (int)op << std::endl;
        exit(2);
    }

    assert(lhs.typ == rhs.typ);
    auto result = temp_operand(lhs.typ);
    current_function().instructions.push_back({ir_op, {lhs, rhs}, result});
    return result;
}

modul::modul(std::string filename)
    : filename{std::move(filename)} {
    functions.emplace(
        "print",
        function_details{std::vector{instruction{operation::syscall,
                                                 {
                                                     operand{"3", integer_type::instance},
                                                     operand{"input", string_type::instance},
                                                     operand{"0", integer_type::instance},
                                                     operand{"0", integer_type::instance},
                                                     operand{"1", integer_type::instance},
                                                 },
                                                 {}},
                                     instruction{operation::ret, {}, {}}},
                         std::vector{id_and_type{"input", string_type::instance}}, "", func_num++});
}

modul::function_details::function_details(const std::vector<id_and_type> & parameters,
                                          const std::optional<std::string> & opt_ret_type,
                                          uint32_t number)
    : parameters{parameters}
    , return_type{opt_ret_type.value_or("")}
    , number{number} {}

type_ptr modul::function_details::generate_type() const {
    std::vector<type_ptr> args;
    for (auto & param : parameters) args.push_back(param.second);
    return std::make_shared<ir::func_type>(std::move(args), ast_to_ir_type(return_type));
}

modul::function_details & modul::current_function() {
    auto iter = functions.find(current_func_name);
    assert(iter != functions.end());
    return iter->second;
}

operand modul::temp_operand(type_ptr type) {
    return {"temp_" + std::to_string(temp_num++), std::move(type)};
}

std::ostream & operator<<(std::ostream & lhs, const ir::modul & rhs) {
    lhs << "File: " << rhs.filename << std::endl;

    for (auto & iter : rhs.functions) {
        lhs << "Function " << iter.first << '\n';
        lhs << "Parameters: (";
        for (auto & param : iter.second.parameters)
            lhs << *param.second << ' ' << param.first << ", ";
        lhs << ")\n";
        lhs << "Returns " << iter.second.return_type << '\n';
        for (auto & inst : iter.second.instructions) lhs << inst << '\n';
        lhs << std::endl;
    }

    return lhs;
}

std::ostream & operator<<(std::ostream & lhs, const ir::instruction & rhs) {
    if (rhs.result.has_value())
        lhs << *rhs.result.value().typ << ' ' << rhs.result.value().name << " = ";
    switch (rhs.op) {
    case operation::syscall:
        lhs << "syscall ";
        break;
    case operation::ret:
        lhs << "ret ";
        break;
    case operation::call:
        lhs << "call ";
        break;
    case operation::add:
        lhs << "add ";
        break;
    case operation::sub:
        lhs << "sub ";
        break;
    case operation::mul:
        lhs << "mul ";
        break;
    case operation::div:
        lhs << "div ";
        break;
    case operation::rem:
        lhs << "rem ";
        break;
    case operation::boolean_and:
        lhs << "boolean_and ";
        break;
    case operation::boolean_or:
        lhs << "boolean_or ";
        break;
    case operation::less_eq:
        lhs << "less_eq ";
        break;
    case operation::less:
        lhs << "less ";
        break;
    case operation::greater_eq:
        lhs << "greater_eq ";
        break;
    case operation::greater:
        lhs << "greater ";
        break;
    case operation::equal:
        lhs << "equal ";
        break;
    case operation::not_equal:
        lhs << "not_equal ";
        break;
    case operation::bit_and:
        lhs << "bit_and ";
        break;
    case operation::bit_or:
        lhs << "bit_or ";
        break;
    case operation::bit_xor:
        lhs << "bit_xor ";
        break;
    case operation::bit_left:
        lhs << "bit_left ";
        break;
    case operation::bit_right:
        lhs << "bit_right ";
        break;
    case operation::assign:
        lhs << "assign ";
        break;
    case operation::boolean_not:
        lhs << "boolean_not ";
        break;
    case operation::negation:
        lhs << "negation ";
        break;
    case operation::bit_not:
        lhs << "bit_not ";
        break;
    }
    for (auto & arg : rhs.args) lhs << *arg.typ << ' ' << arg.name << ", ";
    return lhs;
}

} // namespace ir
