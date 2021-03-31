
%{

%}

%token id string_literal integer_literal float_literal boolean_literal char_literal
%token plus "+" minus "-" divide "/" mult "*" remainder "%"
%token plus_eq "+=" minus_eq "-=" divide_eq "/=" mult_eq "*="
%token bit_and "&" bit_or "|" shift_left "<<" shift_right ">>" bit_not "~" t_xor "^"
%token bit_and_eq "&=" bit_or_eq "|=" shift_left_eq "<<=" shift_right_eq ">>=" t_xor_eq "^="
%token t_and "&&" t_or "||" t_not "!" 
%token less_than "<" greater_than ">" less_than_eq "<=" greater_than_eq ">=" equal "==" not_equal "!="
%token assign "=" colon ":" comma "," semi ";"
%token lparen "(" rparen ")" lbrace "{" rbrace "}" lbrack "[" rbrack "]"

%token func t_for t_while t_else t_if t_return let t_const t_struct
%token i32 i64 f32 f64 t_string t_char t_bool

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

program: top_lvl_item 
	| program top_lvl_item
	;

top_lvl_item: function 
	| const_decl 
	| struct_decl
	;

struct_decl: t_struct id "{" struct_items "}" ;
struct_items: %empty 
	| typed_id ";" struct_items
	;

function: func id param_list opt_typed function_body;

function_body: "=" expr 
	| stmt
	;

param_list: "(" ")" 
	| "("parameters")"
	;

parameters: typed_id 
	| parameters "," typed_id
	;

typed_id: id ":" type ;


type: t_bool
	| t_char 
	| t_string 
	| id 
	| i32 
	| i64 
	| f32 
	| f64
	;

stmt: function_call 
	| assignment 
	| block_stmt 
	| if_stmt 
	| while_stmt 
	| for_stmt 
	| return_stmt 
	| decl_stmt
	;

block_stmt: "{" stmt_list "}" ;

stmt_list: %empty 
	| stmt";" stmt_list
	;

return_stmt: t_return 
	| t_return expr
	;

if_stmt: t_if "(" expr ")" stmt else_block ;

else_block: %empty %prec then
	| t_else stmt
	;

while_stmt: t_while "(" expr ")" stmt ;

for_stmt: t_for "(" assign_or_decl_stmt ";" expr ";" assignment ")" stmt ;

assign_or_decl_stmt: decl_stmt 
	| assignment
	;

decl_stmt: let id opt_typed "=" expr 
	| const_decl
	;

const_decl: t_const id opt_typed "=" expr ;

opt_typed: %empty 
	| ":"type
	;

assignment: lvalue assign_op expr ;

lvalue: id 
	| lvalue "." id
	;

function_call: id "(" args ")" ;

args: %empty 
	| expr 
	| expr "," args
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
	| "-" expr	%prec negate
	| "~" expr
	| t_if "(" expr ")" expr t_else expr
	;

primitive: literal 
	| "(" expr ")" 
	| function_call 
	| struct_creation 
	| lvalue
	;

struct_creation: id "{" field_assignments "}" ;

field_assignments: %empty 
	| id "=" expr 
	| id "=" expr "," field_assignments
	;

assign_op: "=" 
	| "+=" 
	| "-=" 
	| "/=" 
	| "%=" 
	| "<<=" 
	| ">>=" 
	| "&=" 
	| "|=" 
	| "^="
    | "*="
	;

literal: string_literal
	| integer_literal
	| float_literal
	| boolean_literal
	| char_literal
	;
