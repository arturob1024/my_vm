#ifndef MODULE_FORWARD_H
#define MODULE_FORWARD_H

#include <cstdio>
#include <memory>
#include <string>
#include <utility>

class modul;
using module_and_file = std::pair<std::unique_ptr<modul>, FILE *>;
using id_and_type = std::pair<std::string, std::string>;

struct compiled_expr;

#endif
