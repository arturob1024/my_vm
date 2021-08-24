#include "compiler/tokens.hpp"
#include "flex_bison.h"
#include "parser.h"

#include <cstdio>
#include <iostream>

int main(const int arg_count, const char * const * const args) {

    yyin = nullptr;
    if (arg_count > 1) {
        // use the first arg as a input filename
        if (auto * input_file = fopen(args[1], "r"); input_file != nullptr) {
            yyin = input_file;
        } else {
            perror("Opening input");
        }
    }

    // Fallback case: use standard input
    if (yyin == nullptr) {
        puts("Using standard input");
        yyin = stdin;
    }

    switch (const auto yyparse_code = yyparse(); yyparse_code) {
    case 0:
        std::cout << "Parsed " << top_lvl_items.size() << " top level items" << std::endl;
        break;
    case 1:
        puts("Invalid input");
        break;
    case 2:
        puts("Memory exhuasted");
        break;
    default:
        std::cout << "Unknown yyparse code: " << yyparse_code << std::endl;
        exit(0);
    }
}
