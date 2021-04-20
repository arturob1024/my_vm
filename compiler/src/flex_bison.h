#ifndef FLEX_BISON_H
#define FLEX_BISON_H

#include "ast/nodes.h"

using namespace ast;

#include <iostream>
#include <string>
#include <vector>

int yylex();
extern "C" int yywrap();

#endif
