#include "module.h"

#include "ast/nodes.h"

module_and_file modul::open_module(const char * path) {
    return {std::make_unique<modul>(modul{path}), fopen(path, "r")};
}

module_and_file modul::open_stdin() { return {std::make_unique<modul>(modul{"stdin"}), stdin}; }

void modul::add_top_level_item(ast::top_level * top_lvl) { top_lvl_items.emplace_back(top_lvl); }
