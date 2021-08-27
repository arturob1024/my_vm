#include "compiler/tokens.hpp"
#include "flex_bison.h"
#include "module.h"
#include "parser.h"

#include <cassert>
#include <cstdio>
#include <iostream>

std::unique_ptr<modul> current_module;

int main(const int arg_count, const char * const * const args) {

    yyin = nullptr;
    if (arg_count <= 1) {
        // open stdin

        auto [open_module, input] = modul::open_stdin();
        yyin = input;
        current_module = std::move(open_module);
    } else {
        // use the first arg as a input filename
        auto [open_module, input_file] = modul::open_module(args[1]);
        if (input_file != nullptr) {
            yyin = input_file;
            current_module = std::move(open_module);
        } else {
            perror("Opening input");
            exit(1);
        }
    }

    assert(current_module != nullptr);

    switch (const auto yyparse_code = yyparse(); yyparse_code) {
    case 0:
        std::cout << "Parsed " << current_module->top_level_item_count() << " top level items"
                  << std::endl;
        break;
    case 1:
        puts("Invalid input");
        exit(0);
    case 2:
        puts("Memory exhuasted");
        exit(0);
    default:
        std::cout << "Unknown yyparse code: " << yyparse_code << std::endl;
        exit(0);
    }

    current_module->build();

    current_module->write();
}
