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
    used_registers.clear();
}

void modul::call_function(std::string id, std::vector<compiled_expr> args) {
    std::cout << "Calling function " << id << std::endl;
    if (args.size() > 5) {
        std::cout << "Function " << id
                  << " called with >5 arguments. This is currently unsupported." << std::endl;
        exit(2);
    }

    uint8_t arg_reg = 3u;
    for (auto & arg : args) add_instruction(opcode::ori, i_type{arg_reg++, arg.reg_num, 0});

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

compiled_expr modul::compile_literal(const std::string & value, ast::type typ) {
    switch (typ) {
    case ast::type::floating:
        std::cout << "Cannot handle floating point at the moment." << std::endl;
        exit(3);
    case ast::type::string: {
        // save to data segment
        auto str_loc = data_segment_start + data_segment.size();
        assert(str_loc < UINT32_MAX);
        for (auto c : value) data_segment.push_back(c);
        data_segment.push_back(0);
        auto result = alloc_reg();
        if (str_loc <= 0xFFFF) {
            add_instruction(opcode::ori, i_type{result, zero, static_cast<uint16_t>(str_loc)});
        } else {
            add_instruction(opcode::ori,
                            i_type{result, zero, static_cast<uint16_t>(str_loc & 0xFFFF)});
            add_instruction(opcode::lui,
                            i_type{result, result, static_cast<uint16_t>(str_loc >> 16)});
        }
        return {result, "string"};
    }
    default:
        std::cout << "Tried to load " << value << ", which is not currently supported at this time."
                  << std::endl;
        exit(4);
    }
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

uint8_t modul::alloc_reg() {
    auto candidate = static_cast<reg>(s0 + random() % (s19 - s0 + 1));
    while (used_registers.count(candidate) != 0)
        candidate = static_cast<reg>(s0 + random() % (s19 - s0 + 1));
    used_registers.insert(candidate);
    return candidate;
}

modul::function_details::function_details(const std::vector<id_and_type> & params,
                                          const std::optional<std::string> & ret_type,
                                          uint32_t number)
    : parameters{params}
    , return_type{ret_type.value_or("")}
    , number{number} {}

