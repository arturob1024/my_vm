
%{
    #include "flex_bison.h"

    // TODO: Better error handling in the parser

    static void yyerror(const char * msg){
        std::cerr << "Error: " << msg << std::endl;
    }
%}

%union{
    int token;
    assignment_operation assign_op;
    std::string * string;

/* General pointer types */
    expression * expr;
    statement * stmt;
    top_level * top_lvl;

/* Special types */
    function_call * func_call;
    const_decl * const_declaration;
    lvalue * lval;
    typed_id * id_with_type;

/* Sequence types */
    std::vector<statement*>* statements;
    std::vector<expression*>* arguments;
    std::vector<typed_id>* typed_ids;
    std::vector<field_assignment>* field_assignments;
}

/*
A special type is one that could be either one or another general type,
and so has to be declared separately and then casted when appropriate.
*/

/*
Note that the sequence types will be moved from and as such should be deleted.
This is to simplify memory management in the rest of the program.

The same is not the case for other types, as those become owned by their destinations.

A sequence of pointers allows polymorphism, otherwise there is none.
*/

%token <string> id string_literal integer_literal float_literal boolean_literal char_literal
%token <token> plus "+" minus "-" divide "/" mult "*" remainder "%"
%token <token> plus_eq "+=" minus_eq "-=" divide_eq "/=" mult_eq "*=" remainder_eq "%="
%token <token> bit_and "&" bit_or "|" shift_left "<<" shift_right ">>" bit_not "~" t_xor "^"
%token <token> bit_and_eq "&=" bit_or_eq "|=" shift_left_eq "<<=" shift_right_eq ">>=" t_xor_eq "^="
%token <token> t_and "&&" t_or "||" t_not "!"
%token <token> less_than "<" greater_than ">" less_than_eq "<=" greater_than_eq ">=" equal "==" not_equal "!="
%token <token> assign "=" colon ":" comma "," semi ";" dot "."
%token <token> lparen "(" rparen ")" lbrace "{" rbrace "}" lbrack "[" rbrack "]"

%token <token> func t_for t_while t_else t_if t_return let t_const t_struct
%token <string> prim_type

/* tell bison the types of the nonterminals and terminals */
%nterm <assign_op> assign_op
%nterm <string> opt_typed type
%nterm <expr> expr primitive struct_creation literal
%nterm <stmt> oneline_stmt else_block if_stmt return_stmt block_stmt compound_stmt stmt
%nterm <stmt> while_stmt for_stmt assign_or_decl_stmt assignment decl_stmt function_body
%nterm <top_lvl> function struct_decl top_lvl_item
%nterm <func_call> function_call
%nterm <const_declaration> const_decl
%nterm <lval> lvalue
%nterm <id_with_type> typed_id
%nterm <statements> stmt_list
%nterm <arguments> args
%nterm <typed_ids> param_list parameters struct_items
%nterm <field_assignments> field_assignments

%precedence then
%precedence t_else

%left "&&" "||"
%left "&" "|" "^"
%nonassoc equal not_equal
%nonassoc less_than greater_than less_than_eq greater_than_eq
%nonassoc "<<" ">>"
%left "+" "-"
%left "*" "/" "%"
%precedence "!" "~" negate

%start program

%%

/* { $$ = $1; } is the implied action. */

program: top_lvl_item { current_module->add_top_level_item($1); }
    | program top_lvl_item { current_module->add_top_level_item($2); }
    ;

top_lvl_item: function
    | const_decl { $$ = dynamic_cast<top_level *>($1); }
    | struct_decl
    ;

struct_decl: t_struct id lbrace struct_items rbrace { $$ = new struct_decl{$2, std::move(*$4)}; delete $4; }
           ;

struct_items: %empty             { $$ = new std::vector<typed_id>; }
    | typed_id semi struct_items { $$ = $3; $$->push_back(std::move(*$1)); delete $1; }
    ;

function: func id param_list opt_typed function_body
        { $$ = new function_decl{$2, std::move(*$3), $4, $5}; delete $3; }
        ;

function_body: "=" expr { $$ = new return_stmt{$2}; }
    | stmt
    ;

param_list: "(" ")"      { $$ = new std::vector<typed_id>; }
    | "(" parameters ")" { $$ = $2; }
    ;

parameters: typed_id { $$ = new std::vector<typed_id>; $$->push_back(std::move(*$1)); delete $1; }
    | parameters "," typed_id { $$ = $1; $$->push_back(std::move(*$3)); delete $3; }
    ;

typed_id: id ":" type { $$ = new typed_id{$1, $3}; }
        ;

type: prim_type
    | id
    ;

stmt: oneline_stmt semi
    | compound_stmt
    ;

oneline_stmt: function_call { $$ = dynamic_cast<statement*>($1); }
    | assign_or_decl_stmt
    | return_stmt
    ;

compound_stmt: block_stmt
    | if_stmt
    | while_stmt
    | for_stmt
    ;

block_stmt: lbrace stmt_list rbrace { $$ = new block_stmt{std::move(*$2)}; delete $2; }
          ;

stmt_list: %empty { $$ = new std::vector<statement*>; }
    | stmt stmt_list { $$ = $2; $$->push_back($1); }
    ;

return_stmt: t_return { $$ = new return_stmt; }
    | t_return expr { $$ = new return_stmt{$2}; }
    ;

if_stmt: t_if "(" expr ")" stmt else_block { $$ = new if_stmt{$3, $5, $6}; }
       ;

else_block: %empty %prec then { $$ = nullptr; }
    | t_else stmt { $$ = $2; }
    ;

while_stmt: t_while "(" expr ")" stmt { $$ = new while_stmt{$3, $5}; }
          ;

for_stmt: t_for "(" assign_or_decl_stmt semi expr semi assignment ")" stmt
        { $$ = new for_stmt{$3, $5, $7, $9}; }
        ;

assign_or_decl_stmt: decl_stmt
    | assignment
    ;

decl_stmt: let id opt_typed "=" expr { $$ = new let_stmt{$2, $3, $5}; }
    | const_decl { $$ = dynamic_cast<statement*>($1); }
    ;

const_decl: t_const id opt_typed "=" expr { $$ = new const_decl{$2, $3, $5}; }
          ;

opt_typed: %empty { $$ = nullptr; }
    | ":" type { $$ = $2; }
    ;

assignment: lvalue assign_op expr { $$ = new assignment{$1, $2, $3}; }
          ;

lvalue: id          { $$ = new lvalue{$1}; }
    | lvalue "." id { $$ = new lvalue{$3, $1}; }
    ;

function_call: id "(" args ")" { $$ = new function_call{$1, std::move(*$3)}; delete $3; }
             ;

args: %empty        { $$ = new std::vector<expression*>{}; }
    | expr          { $$ = new std::vector{$1}; }
    | expr "," args { $$ = $3; $$->push_back($1); }
    ;

expr: primitive
    | expr "+" expr                         { $$ = new binary_expr{$1, binary_operation::add, $3}; }
    | expr "-" expr                         { $$ = new binary_expr{$1, binary_operation::sub, $3}; }
    | expr "*" expr                         { $$ = new binary_expr{$1, binary_operation::mul, $3}; }
    | expr "/" expr                         { $$ = new binary_expr{$1, binary_operation::div, $3}; }
    | expr "&&" expr                        { $$ = new binary_expr{$1, binary_operation::boolean_and, $3}; }
    | expr "||" expr                        { $$ = new binary_expr{$1, binary_operation::boolean_or, $3}; }
    | expr "<=" expr                        { $$ = new binary_expr{$1, binary_operation::less_eq, $3}; }
    | expr "<" expr                         { $$ = new binary_expr{$1, binary_operation::less, $3}; }
    | expr ">=" expr                        { $$ = new binary_expr{$1, binary_operation::greater_eq, $3}; }
    | expr ">" expr                         { $$ = new binary_expr{$1, binary_operation::greater, $3}; }
    | expr "==" expr                        { $$ = new binary_expr{$1, binary_operation::equal, $3}; }
    | expr "!=" expr                        { $$ = new binary_expr{$1, binary_operation::not_equal, $3}; }
    | expr "&" expr                         { $$ = new binary_expr{$1, binary_operation::bit_and, $3}; }
    | expr "|" expr                         { $$ = new binary_expr{$1, binary_operation::bit_or, $3}; }
    | expr "<<" expr                        { $$ = new binary_expr{$1, binary_operation::bit_right, $3}; }
    | expr ">>" expr                        { $$ = new binary_expr{$1, binary_operation::bit_left, $3}; }
    | expr "^" expr                         { $$ = new binary_expr{$1, binary_operation::bit_xor, $3}; }
    | expr "%" expr                         { $$ = new binary_expr{$1, binary_operation::rem, $3}; }
    | "!" expr                              { $$ = new unary_expr{unary_operation::boolean_not, $2}; }
    | "-" expr    %prec negate              { $$ = new unary_expr{unary_operation::negation, $2}; }
    | "~" expr                              { $$ = new unary_expr{unary_operation::bit_not, $2}; }
    | t_if "(" expr ")" expr t_else expr    { $$ = new if_expr{$3, $5, $7}; }
    ;

primitive: literal
    | "(" expr ")"      { $$ = $2; }
    | function_call     { $$ = dynamic_cast<expression*>($1); }
    | struct_creation
    | lvalue            { $$ = dynamic_cast<expression*>($1); }
    ;

struct_creation: id lbrace field_assignments rbrace { $$ = new struct_init{$1, std::move(*$3)}; delete $3; }
               ;

field_assignments: %empty               { $$ = new std::vector<field_assignment>; }
    | id "=" expr                       { $$ = new std::vector<field_assignment>; $$->emplace_back($1, $3); }
    | id "=" expr "," field_assignments { $$ = $5; $$->emplace_back($1, $3); }
    ;

assign_op: "=" { $$ = assignment_operation::assign; }
    | "+="     { $$ = assignment_operation::add; }
    | "-="     { $$ = assignment_operation::sub; }
    | "/="     { $$ = assignment_operation::div; }
    | "%="     { $$ = assignment_operation::remainder; }
    | "<<="    { $$ = assignment_operation::bit_left; }
    | ">>="    { $$ = assignment_operation::bit_right; }
    | "&="     { $$ = assignment_operation::bit_and; }
    | "|="     { $$ = assignment_operation::bit_or; }
    | "^="     { $$ = assignment_operation::bit_xor; }
    | "*="     { $$ = assignment_operation::mul; }
    ;

literal: string_literal { $$ = new literal{$1, type::string}; }
    | integer_literal   { $$ = new literal{$1, type::integer}; }
    | float_literal     { $$ = new literal{$1, type::floating}; }
    | boolean_literal   { $$ = new literal{$1, type::boolean}; }
    | char_literal      { $$ = new literal{$1, type::character}; }
    ;
