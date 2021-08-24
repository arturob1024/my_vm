#include "nodes.h"

#include <ostream>

std::ostream & operator<<(std::ostream & lhs, const typed_id & rhs) {
    lhs << rhs.id << " : " << rhs.type;
}

std::ostream & operator<<(std::ostream & lhs, const field_assignment & rhs) {
    lhs << rhs.id << " : " << *rhs.expr;
}
