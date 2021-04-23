
%{
    #include "flex_bison.h"

    static void yyerror(const char * msg){
        std::cerr << "Error: " << msg << std::endl;
    }
%}

%union{
    int token;
    assignment::operation assign_op;
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
%nterm <stmt> stmt else_block if_stmt return_stmt block_stmt
%nterm <stmt> while_stmt for_stmt assign_or_decl_stmt assignment decl_stmt
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

program: top_lvl_item
    | program top_lvl_item
    ;

top_lvl_item: function
    | const_decl
    | struct_decl
    ;

struct_decl: t_struct id lbrace struct_items rbrace
           ;

struct_items: %empty             { $$ = new std::vector<typed_id>; }
    | typed_id semi struct_items { $$ = $3; $$->push_back(std::move(*$1)); delete $1; }
    ;

function: func id param_list opt_typed function_body
        ;

function_body: "=" expr
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

stmt: function_call { $$ = dynamic_cast<statement*>($1); }
    | assignment
    | block_stmt
    | if_stmt
    | while_stmt
    | for_stmt
    | return_stmt
    ;

block_stmt: lbrace stmt_list rbrace { $$ = new block_stmt{std::move(*$2)}; delete $2; }
          ;

stmt_list: %empty { $$ = new std::vector<statement*>{}; }
    | stmt semi stmt_list { $$ = $3; $$->push_back($1); }
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

lvalue: id
    | lvalue "." id
    ;

function_call: id "(" args ")" { $$ = new function_call{$1, std::move(*$3)}; delete $3; }
             ;

args: %empty        { $$ = new std::vector<expression*>{}; }
    | expr          { $$ = new std::vector{$1}; }
    | expr "," args { $$ = $3; $$->push_back($1); }
    ;

expr: primitive
    | expr "+" expr
    | expr "-" expr
    | expr "*" expr
    | expr "/" expr
    | expr "&&" expr
    | expr "||" expr
    | expr "<=" expr
    | expr "<" expr
    | expr ">=" expr
    | expr ">" expr
    | expr "==" expr
    | expr "!=" expr
    | expr "&" expr
    | expr "|" expr
    | expr "<<" expr
    | expr ">>" expr
    | expr "^" expr
    | expr "%" expr
    | "!" expr
    | "-" expr    %prec negate
    | "~" expr
    | t_if "(" expr ")" expr t_else expr { $$ = new if_expr{$3, $5, $7}; }
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

assign_op: "=" { $$ = assignment::operation::assign; }
    | "+="     { $$ = assignment::operation::add; }
    | "-="     { $$ = assignment::operation::sub; }
    | "/="     { $$ = assignment::operation::div; }
    | "%="     { $$ = assignment::operation::remainder; }
    | "<<="    { $$ = assignment::operation::bit_left; }
    | ">>="    { $$ = assignment::operation::bit_right; }
    | "&="     { $$ = assignment::operation::bit_and; }
    | "|="     { $$ = assignment::operation::bit_or; }
    | "^="     { $$ = assignment::operation::bit_xor; }
    | "*="     { $$ = assignment::operation::mul; }
    ;

literal: string_literal { $$ = new literal{$1, literal::type::string}; }
    | integer_literal   { $$ = new literal{$1, literal::type::integer}; }
    | float_literal     { $$ = new literal{$1, literal::type::floating}; }
    | boolean_literal   { $$ = new literal{$1, literal::type::boolean}; }
    | char_literal      { $$ = new literal{$1, literal::type::character}; }
    ;
