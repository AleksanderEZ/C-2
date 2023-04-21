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
%left ASTERISK DIVISION

%% 
program: | program instruction | program function_declaration;
instructions: | instructions instruction;
instruction: simple_instruction_type SEMICOLON
              | complex_instruction_type;
simple_instruction_type: | simple_declaration | function_call | assignment | return | BREAK | CONTINUE;
complex_instruction_type: control | complex_declaration | instruction_block;
instruction_block: OPEN_CURLY instructions CLOSE_CURLY;

control: for | while | if;
for: for_header instruction;
for_header: FOR OPEN_PARENTHESIS first_part_for SEMICOLON condition SEMICOLON third_part_for CLOSE_PARENTHESIS;
first_part_for: | arithmetical_assignment | type identifier ASSIGNMENT expression;
third_part_for: | assignment;
while: while_header instruction;
while_header: WHILE OPEN_PARENTHESIS condition CLOSE_PARENTHESIS;
if: if_header instruction
  | if_header instruction else;
if_header: IF OPEN_PARENTHESIS condition CLOSE_PARENTHESIS;
else: ELSE instruction;

condition: OPEN_PARENTHESIS condition CLOSE_PARENTHESIS
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
          | expression LESSER_EQUALS expression;
assignment: arithmetical_assignment
           | identifier ASSIGNMENT STRING_VALUE
           | identifier ASSIGNMENT array
           | identifier ASSIGNMENT reference_pointer
           | identifier ASSIGNMENT dereference_pointer
           | IDENTIFIER array_size ASSIGNMENT expression
reference_pointer: ASTERISK IDENTIFIER;
dereference_pointer: AMPERSAND IDENTIFIER;
array: OPEN_CURLY value_list CLOSE_CURLY;
arithmetical_assignment: identifier ASSIGNMENT expression;
complex_declaration: function_declaration;
simple_declaration: type identifier
                  | type identifier ASSIGNMENT expression
                  | array_declaration;
function_declaration: function_header instruction_block;
function_header: type function_subheader
                | VOID function_subheader;
function_subheader: identifier OPEN_PARENTHESIS parameters CLOSE_PARENTHESIS
                  | identifier OPEN_PARENTHESIS CLOSE_PARENTHESIS;
parameters: type identifier | parameters COMMA type identifier;
array_declaration: type IDENTIFIER array_size
                  | type IDENTIFIER OPEN_SQUARE CLOSE_SQUARE ASSIGNMENT array;
array_size: OPEN_SQUARE INT_VALUE CLOSE_SQUARE;
identifier: IDENTIFIER | ASTERISK IDENTIFIER;
type: INT
    | CHAR
    | FLOAT;
expression: OPEN_PARENTHESIS expression CLOSE_PARENTHESIS
           | arithmetical_expression
           | pointer_expression
           | value
           | identifier
           | function_call
           | identifier OPEN_SQUARE expression CLOSE_SQUARE;

arithmetical_expression:
             expression ADDITION expression
           | expression SUBTRACTION expression 
           | expression DIVISION expression 
           | expression ASTERISK expression 
           | expression MODULUS expression
           | identifier INCREMENT
           | identifier array_size INCREMENT
           | identifier DECREMENT
           | identifier array_size DECREMENT
           | INCREMENT identifier
           | INCREMENT identifier array_size
           | DECREMENT identifier;
           | DECREMENT identifier array_size;
pointer_expression: AMPERSAND expression
           | ASTERISK expression;
function_call: IDENTIFIER OPEN_PARENTHESIS arguments CLOSE_PARENTHESIS 
             | IDENTIFIER OPEN_PARENTHESIS CLOSE_PARENTHESIS 
             | MALLOC OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS
             | SIZEOF OPEN_PARENTHESIS type CLOSE_PARENTHESIS
             | PRINTF OPEN_PARENTHESIS STRING_VALUE CLOSE_PARENTHESIS
             | PRINTF OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS
             | PRINTF OPEN_PARENTHESIS STRING_VALUE COMMA arguments CLOSE_PARENTHESIS
             | PRINTF OPEN_PARENTHESIS IDENTIFIER COMMA arguments CLOSE_PARENTHESIS;
arguments: arguments COMMA expression
          | arguments COMMA STRING_VALUE;
value_list: expression | value_list COMMA expression;
value: INT_VALUE 
     | FLOAT_VALUE 
     | CHAR_VALUE;
return: RETURN expression
      | RETURN;
%%

int main(int argc, char** argv) {
  if (argc>1) yyin=fopen(argv[1],"r");
  yyparse();
}

void yyerror(char* message) {
  printf("Error in line %i: %s \n", yylineno, message);
}
