#ifndef FLEX_BISON_H
#define FLEX_BISON_H

#include "ast/nodes.h"

#include <iostream>
#include <string>
#include <vector>

using namespace ast;

// TODO: Support multiple modules
extern std::unique_ptr<modul> current_module;

int yylex();
extern "C" int yywrap();

#endif
