#ifndef MODULE_FORWARD_H
#define MODULE_FORWARD_H

#include <cstdio>
#include <string>
#include <utility>

namespace bytecode {
class modul;
using id_and_type = std::pair<std::string, std::string>;

struct compiled_expr;
} // namespace bytecode

#endif
