#ifndef FLEX_BISON_H
#define FLEX_BISON_H

#include "ast/nodes.h"

#include <iostream>
#include <string>

int yylex();
extern "C" int yywrap();

#endif
