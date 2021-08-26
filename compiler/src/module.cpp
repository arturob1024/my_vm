#include "module.h"

#include "ast/nodes.h"

#include <cassert>
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

void modul::register_function(std::string id, const std::vector<ast::typed_id> &,
                              const std::optional<std::string> &, ast::statement &) {
    std::cout << "Registered a function named " << id << std::endl;
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
