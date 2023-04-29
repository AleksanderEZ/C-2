%{
#include <stdio.h>
extern FILE *yyin;
extern int yylineno;
int yydebug = 1;
void yyerror(char*);
%}

%union {float real; int integer; char characater; char* string}
%token COMMENT IDENTIFIER ERROR
%token <string> STRING_VALUE
%token <integer> INT_VALUE 
%token <real> FLOAT_VALUE 
%token <character> CHAR_VALUE
%token ADDITION INCREMENT SUBTRACTION DECREMENT DIVISION ASTERISK AMPERSAND MODULUS EQUALS NOT_EQUALS GREATER GREATER_EQUALS LESSER LESSER_EQUALS AND OR NEGATOR
%token COMMA SEMICOLON ASSIGNMENT OPEN_CURLY CLOSE_CURLY OPEN_PARENTHESIS CLOSE_PARENTHESIS OPEN_SQUARE CLOSE_SQUARE
%token INT CHAR FLOAT VOID WHILE FOR BREAK CONTINUE RETURN IF ELSE SIZEOF MALLOC PRINTF

%left EQUALS NOT_EQUALS GREATER GREATER_EQUALS LESSER LESSER_EQUALS NEGATOR AND OR
%left ADDITION SUBTRACTION
%left ASTERISK DIVISION MODULUS
%left OPEN_PARENTHESIS CLOSE_PARENTHESIS

%% 
program
  : 
  | program instruction 
  | program function_declaration;

instructions
  : 
  | instructions instruction
  ;

instruction 
  : simple_instruction_type SEMICOLON
  | complex_instruction_type
  ;

simple_instruction_type
  : 
  | simple_declaration 
  | function_call 
  | assignment 
  | return 
  | BREAK 
  | CONTINUE
  ;

simple_declaration
  : type IDENTIFIER
  | type assignment
  | array_declaration
  ;

complex_instruction_type
  : control 
  | instruction_block
  ;

instruction_block
  : OPEN_CURLY instructions CLOSE_CURLY
  ;

control
  : for 
  | while 
  | if
  ;

for
  : for_header instruction
  ;

for_header
  : FOR OPEN_PARENTHESIS first_part_for SEMICOLON condition SEMICOLON third_part_for CLOSE_PARENTHESIS
  ;

first_part_for
  : 
  | arithmetical_assignment 
  | type IDENTIFIER ASSIGNMENT expression
  ;

third_part_for
  : 
  | assignment
  ;

while
  : while_header instruction
  ;

while_header
  : WHILE OPEN_PARENTHESIS condition CLOSE_PARENTHESIS
  ;

if
  : if_header simple_instruction_type SEMICOLON
  | if_header simple_instruction_type SEMICOLON else
  | if_header instruction_block
  | if_header instruction_block else
  ;

if_header
  : IF OPEN_PARENTHESIS condition CLOSE_PARENTHESIS
  ;

else
  : ELSE instruction
  ;

condition
  : OPEN_PARENTHESIS condition CLOSE_PARENTHESIS
  | condition AND condition
  | condition OR condition
  | NEGATOR condition
  | '1'
  | '0' 
  | expression EQUALS expression 
  | expression NOT_EQUALS expression 
  | expression GREATER expression
  | expression GREATER_EQUALS expression
  | expression LESSER expression
  | expression LESSER_EQUALS expression
  ;

assignment
  : arithmetical_assignment
  | IDENTIFIER array_index ASSIGNMENT expression
  ;

arithmetical_assignment
  : IDENTIFIER ASSIGNMENT expression
  ;

expression
  : OPEN_PARENTHESIS expression CLOSE_PARENTHESIS
  | value
  | function_call
  | IDENTIFIER array_index
  | OPEN_PARENTHESIS type CLOSE_PARENTHESIS expression
  | IDENTIFIER
  | AMPERSAND IDENTIFIER
  | ASTERISK IDENTIFIER
  | expression ADDITION expression
  | expression SUBTRACTION expression 
  | expression DIVISION expression 
  | expression ASTERISK expression 
  | expression MODULUS expression
  | IDENTIFIER INCREMENT
  | IDENTIFIER array_index INCREMENT
  | IDENTIFIER DECREMENT
  | IDENTIFIER array_index DECREMENT
  | INCREMENT IDENTIFIER
  | INCREMENT IDENTIFIER array_index
  | DECREMENT IDENTIFIER
  | DECREMENT IDENTIFIER array_index
  ;

function_call
  : IDENTIFIER OPEN_PARENTHESIS arguments CLOSE_PARENTHESIS 
  | IDENTIFIER OPEN_PARENTHESIS CLOSE_PARENTHESIS 
  | MALLOC OPEN_PARENTHESIS expression CLOSE_PARENTHESIS
  | SIZEOF OPEN_PARENTHESIS type CLOSE_PARENTHESIS
  | PRINTF OPEN_PARENTHESIS expression CLOSE_PARENTHESIS
  | PRINTF OPEN_PARENTHESIS STRING_VALUE COMMA arguments CLOSE_PARENTHESIS
  | PRINTF OPEN_PARENTHESIS IDENTIFIER COMMA arguments CLOSE_PARENTHESIS
  ;

arguments
  : arguments COMMA expression
  | expression 
  ;

return
  : RETURN expression
  | RETURN
  ;

array
  : OPEN_CURLY value_list CLOSE_CURLY
  ;

value_list
  : expression
  | value_list COMMA expression
  ;

function_declaration
  : function_header instruction_block
  ;

function_header
  : type function_subheader
  | VOID function_subheader
  ;

function_subheader
  : IDENTIFIER OPEN_PARENTHESIS parameters CLOSE_PARENTHESIS
  | IDENTIFIER OPEN_PARENTHESIS CLOSE_PARENTHESIS
  ;

parameters
  : type IDENTIFIER 
  | parameters COMMA type IDENTIFIER
  ;

array_declaration
  : type IDENTIFIER array_index
  | type IDENTIFIER OPEN_SQUARE CLOSE_SQUARE ASSIGNMENT array
  ;

array_index
  : OPEN_SQUARE expression CLOSE_SQUARE
  ;

type
  : pointer
  | INT
  | CHAR
  | FLOAT
  ;

pointer
  : ASTERISK type
  ;

value
  : INT_VALUE 
  | FLOAT_VALUE 
  | CHAR_VALUE
  | STRING_VALUE
  ;
%%

int main(int argc, char** argv) {
  if (argc>1) yyin=fopen(argv[1],"r");
  yyparse();
}

void yyerror(char* message) {
  printf("Error in line %i: %s \n", yylineno, message);
}
