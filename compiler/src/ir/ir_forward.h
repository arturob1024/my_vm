#ifndef IR_FORWARD_H
#define IR_FORWARD_H

namespace ir {

enum class operation {
    call,
    ret,
    syscall,
    add,
    sub,
    mul,
    div,
    rem,
    boolean_and,
    boolean_or,
    less_eq,
    less,
    greater_eq,
    greater,
    equal,
    not_equal,
    bit_and,
    bit_or,
    bit_xor,
    bit_left,
    bit_right,
    assign,
    boolean_not,
    negation,
    bit_not,
};

struct instruction;
struct operand;
struct modul;

} // namespace ir

#endif
