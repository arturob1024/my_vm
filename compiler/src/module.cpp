#include "module.h"

#include "ast/nodes.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

module_and_file modul::open_module(const char * path) {
    return {std::make_unique<modul>(modul{path}), fopen(path, "r")};
}

module_and_file modul::open_stdin() { return {std::make_unique<modul>(modul{"stdin"}), stdin}; }

void modul::add_top_level_item(ast::top_level * top_lvl) { top_lvl_items.emplace_back(top_lvl); }

void modul::register_global(std::string id, const std::optional<std::string> &, ast::expression &,
                            bool constant) {
    std::cout << "Registered a " << (constant ? "constant" : "mutable") << " global named " << id
              << std::endl;
}

void modul::register_function(std::string id, const std::vector<ast::typed_id> & params,
                              const std::optional<std::string> & ret_type, ast::statement & body) {
    std::cout << "Registered a function named " << id << std::endl;

    std::vector<id_and_type> typed_ids;
    for (auto & typed_id : params) typed_ids.push_back(typed_id.id_and_type());

    functions.emplace(id, function_details{std::move(typed_ids), ret_type, func_num++});
    current_function = std::move(id);
    body.build(*this);
    current_function.clear();
}

void modul::call_function(std::string id, std::vector<compiled_expr> args) {
    std::cout << "Calling function " << id << std::endl;
    if (args.size() > 5) {
        std::cout << "Function " << id
                  << " called with >5 arguments. This is currently unsupported." << std::endl;
        exit(2);
    }

    uint8_t arg_reg = 3u;
    for (auto & arg : args)
        add_instruction(opcode::ori, i_type{arg_reg++, arg.register_number(), 0});

    add_instruction(opcode::jal,
                    j_type{31, [this, &id]() -> uint32_t {
                               if (auto iter = functions.find(id); iter != functions.end()) {
                                   return iter->second.number;
                               } else {
                                   std::cout << "Function " << id
                                             << " needs to be defined before it is called."
                                             << std::endl;
                                   exit(3);
                               }
                           }()});
}

void modul::register_struct(std::string id, const std::vector<ast::typed_id> &) {
    std::cout << "Registered a struct named " << id << std::endl;
}

void modul::build() {

    for (auto & top_lvl : top_lvl_items) {
        assert(top_lvl != nullptr);
        top_lvl->build(*this);
    }
}

void modul::add_instruction(opcode op, std::variant<r_type, i_type, j_type, s_type> && data) {

    auto iter = functions.find(current_function);
    assert(iter != functions.end());

    iter->second.instructions.emplace_back(op, std::move(data));
}

modul::function_details::function_details(const std::vector<id_and_type> & params,
                                          const std::optional<std::string> & ret_type,
                                          uint32_t number)
    : parameters{params}
    , return_type{ret_type.value_or("")}
    , number{number} {}

