#ifndef FLEX_BISON_H
#define FLEX_BISON_H

#include "ast/nodes.h"

using namespace ast;

#include <iostream>
#include <string>
#include <vector>

// TODO: Support multiple modules
inline static std::vector<std::unique_ptr<top_level>> top_lvl_items;

int yylex();
extern "C" int yywrap();

#endif
