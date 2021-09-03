#include "ir.h"

#include "ast/nodes.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

namespace ir {
// Top level item compilation

void modul::register_global(std::string, std::optional<std::string>, ast::expression &, bool) {}
void modul::register_function(std::string id, const std::vector<ast::typed_id> & params,
                              const std::optional<std::string> & type, ast::statement & body) {

    assert(functions.find(id) == functions.end());

    std::vector<id_and_type> parameters;
    for (auto & param : params) parameters.push_back(param.id_and_type());
    functions.insert_or_assign(
        id, function_details{{}, std::move(parameters), type.value_or(""), func_num++});
    current_func_name = id;
    (void)body;
    // body.build(*this);
    current_func_name.clear();
}

void modul::register_struct(std::string, const std::vector<ast::typed_id> &) {}

// Statment compilation

void modul::call_function(std::string id, std::vector<operand> args) {

    assert(functions.find(id) != functions.end());

    args.insert(args.begin(), {id, "function"});
    current_function().instructions.emplace_back(operation::call, args, std::nullopt);
}

// Expression compilation

operand modul::compile_literal(const std::string & value, ast::type typ) {
    switch (typ) {
    case ast::type::string:
        return {value, "string"};
    case ast::type::integer:
        return {value, "integer"};
    case ast::type::floating:
        return {value, "floating"};
    case ast::type::character:
        return {value, "character"};
    case ast::type::boolean:
        return {value, "boolean"};
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

    assert(lhs.type == rhs.type);
    auto result = temp_operand(lhs.type);
    current_function().instructions.push_back({ir_op, {lhs, rhs}, result});
    return result;
}

modul::function_details::function_details(const std::vector<id_and_type> & parameters,
                                          const std::optional<std::string> & opt_ret_type,
                                          uint32_t number)
    : parameters{parameters}
    , return_type{opt_ret_type.value_or("")}
    , number{number} {}

modul::function_details & modul::current_function() {
    auto iter = functions.find(current_func_name);
    assert(iter != functions.end());
    return iter->second;
}

operand modul::temp_operand(std::string type) {
    return {"temp_" + std::to_string(temp_num++), std::move(type)};
}

} // namespace ir
