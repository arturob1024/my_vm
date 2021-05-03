%{

#include "flex_bison.h"
#include "parser.h"

static void save_token(){
    yylval.string = new std::string{yytext, static_cast<size_t>(yyleng)};
}

template<typename T>
auto token(T t){
    yylval.token = t;
    return t;
}

extern "C" int yywrap(){ return 1; /* To prevent reading a second file*/ }

%}

%option nounput
%option header="tokens.hpp"

%%

    /* ignore whitespace */
[ \t\n]+    {}

    /* ignore comments */
\/\/[^\r\n]*    {}

    /* Math symbols */
"+"     return token(plus);
"-"     return token(minus);
"*"     return token(mult);
"/"     return token(divide);
"%"     return token(remainder);

"+="     return token(plus_eq);
"-="     return token(minus_eq);
"*="     return token(mult_eq);
"/="     return token(divide_eq);
"%="     return token(remainder_eq);

    /* Bitwise symbols */
"&"     return token(bit_and);
"^"     return token(t_xor);
"|"     return token(bit_or);
"<<"     return token(shift_left);
">>"     return token(shift_right);
"~"     return token(bit_not);

"&="     return token(bit_and_eq);
"^="     return token(t_xor_eq);
"|="     return token(bit_or_eq);
"<<="     return token(shift_left_eq);
">>="     return token(shift_right_eq);

    /* Comparisons and booleans */
"&&"|"and"     return token(t_and);
"||"|"or"     return token(t_or);
"!"|"not"     return token(t_not);

"<"     return token(less_than);
">"     return token(greater_than);
"<="     return token(less_than_eq);
">="     return token(greater_than_eq);
"=="     return token(equal);
"!="     return token(not_equal);

    /* Paired symbols */
"("     return token(lparen);
")"     return token(rparen);
"{"     return token(lbrace);
"}"     return token(rbrace);
"["     return token(lbrack);
"]"     return token(rbrack);

    /* Misc symbols */
":"     return token(colon);
";"     return token(semi);
","     return token(comma);
"."     return token(dot);
"="     return token(assign);

    /* Words */
"func"     return token(func);
"for"     return token(t_for);
"while"     return token(t_while);
"else"     return token(t_else);
"if"     return token(t_if);
"return"     return token(t_return);
"let"     return token(let);
"const"     return token(t_const);
"struct"     return token(t_struct);
"i32"       { save_token(); return prim_type; }
"i64"       { save_token(); return prim_type; }
"f32"       { save_token(); return prim_type; }
"f64"       { save_token(); return prim_type; }
"char"      { save_token(); return prim_type; }
"string"    { save_token(); return prim_type; }
"bool"      { save_token(); return prim_type; }

    /* Literals */

"true"|"false"  { save_token(); return boolean_literal; }

"0"|([1-9][0-9_]*)|(0x[0-9a-fA-F_]+) { save_token(); return integer_literal; }

(0|[1-9][0-9]*)\.[0-9]+     { save_token(); return float_literal; }

\"(\\.|[^\"])*\"            { save_token(); return string_literal; }

'(.|\\.)'                   { save_token(); return char_literal; }

    /* Identifier */
[a-zA-Z_][a-zA-Z0-9_]*      { save_token(); return id; }

    /* TODO: Error case.
        Currently, flex will print any unmatched character to stdout as its default rule.
        This is not what we want. However, no good implementation came to mind.
    */
