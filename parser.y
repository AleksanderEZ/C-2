%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symbol_table.h"
#include "code_generation.h"
#include "Q/Qlib.h"

extern FILE* yyin;
extern int yylineno;
int yydebug = 1;
void yyerror(char*);

enum RegType variableSwitch = globalVariable;
struct Reg* voidType;

void initST() {
  char* voidString = strdup("void");
  char* intString = strdup("int");
  char* floatString = strdup("float");
  char* charString = strdup("char");

  newReg(voidString, type, NULL, 0);
  voidType = getTop();
  newReg(intString, type, NULL, 0);
  newReg(floatString, type, NULL, 0);
  newReg(charString, type, NULL, 0);
}

void dummyReg() {
  char* openBlock;
  variableSwitch = localVariable;
  openBlock = strdup("openBlock");
  newReg(openBlock, function, voidType, yylineno);
}

void declaration(char* typeName, char* name, int line) {
  struct Reg* t = searchRegType(typeName, type);
  if (t == NULL || t == voidType) yyerror("Type does not exist or invalid type");
  newReg(name, variableSwitch, t, line);
}

void functionDeclaration(char* typeName, char* name, int line) {
  struct Reg* t = searchRegType(typeName, type);
  if (t == NULL) yyerror("Type does not exist");
  char* functionName = strtok(name, "(");
  char* definitiveFunctionName = strdup(functionName);
  newReg(definitiveFunctionName, function, t, line);

  char* tokens[10];
  char* token = strtok(NULL, ",");
  if (token == NULL) return;
  char* definitiveToken = strdup(token);
  int i = 0;
  while (token != NULL) {
    definitiveToken = strdup(token);
    tokens[i] = definitiveToken;
    printf("%s\n", tokens[i]);
    i++;
    token = strtok(NULL, ",");
  }

  variableSwitch = localVariable;
  char* type;
  char* definitiveType;
  char* parameterName;
  char* definitiveParameterName;
  for(int j = 0; j < i; j++){
    type = strtok(tokens[j], " ");
    parameterName = strtok(NULL, " ");
    definitiveType = strdup(type);
    definitiveParameterName = strdup(parameterName);
    declaration(definitiveType, definitiveParameterName, line);
    free(tokens[j]);
  }
}

void checkFunExists(char* name) {
  if (searchRegType(name, function) == NULL) yyerror("Function has not been declared");
}

void checkVarExists(char* name) {
  if (searchRegType(name, globalVariable) == NULL && searchRegType(name, localVariable) == NULL) yyerror("Variable has not been declared");
}
%}

%union {float real; int integer; char character; char* string}
%token <string> IDENTIFIER
%token <string> STRING_VALUE
%token <string> SIZEOF
%token <string> MALLOC
%token <string> PRINTF
%token <integer> INT_VALUE
%token <real> FLOAT_VALUE
%token <character> CHAR_VALUE
%token <string> INT
%token <string> CHAR
%token <string> FLOAT
%token <string> VOID
%token ADDITION INCREMENT SUBTRACTION DECREMENT DIVISION MODULUS EQUALS NOT_EQUALS GREATER GREATER_EQUALS LESSER LESSER_EQUALS AND OR NEGATOR ASTERISK AMPERSAND
%token COMMA SEMICOLON ASSIGNMENT OPEN_CURLY CLOSE_CURLY OPEN_PARENTHESIS CLOSE_PARENTHESIS OPEN_SQUARE CLOSE_SQUARE
%token WHILE FOR BREAK CONTINUE RETURN IF ELSE ERROR

%type <string> type
%type <string> function_subheader
%type <string> parameters
%type <string> arguments

%type <string> expression
%type <string> function_call

%left EQUALS NOT_EQUALS GREATER GREATER_EQUALS LESSER LESSER_EQUALS NEGATOR AND OR
%left ADDITION SUBTRACTION
%left ASTERISK DIVISION MODULUS
%left OPEN_PARENTHESIS CLOSE_PARENTHESIS

%%

raiz: { qInit(); } program { qEnd(); };

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
  | expression 
  | assignment 
  | return 
  | BREAK 
  | CONTINUE
  ;

simple_declaration
  : type IDENTIFIER { declaration($1, $2, yylineno); } 
  | type assignment
  | array_declaration
  ;

complex_instruction_type
  : control 
  | instruction_block
  ;

instruction_block
  : OPEN_CURLY { variableSwitch = localVariable; } instructions CLOSE_CURLY {closeBlock(); variableSwitch = globalVariable; }
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
  : FOR { dummyReg(); } OPEN_PARENTHESIS first_part_for SEMICOLON condition SEMICOLON third_part_for CLOSE_PARENTHESIS
  ;

first_part_for
  : 
  | arithmetical_assignment 
  | type IDENTIFIER ASSIGNMENT expression { variableSwitch = localVariable; declaration($1, $2, yylineno); variableSwitch = globalVariable; }
  ;

third_part_for
  : 
  | assignment
  ;

while
  : while_header instruction
  ;

while_header
  : WHILE { dummyReg(); } OPEN_PARENTHESIS condition CLOSE_PARENTHESIS
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
  | IDENTIFIER array_index ASSIGNMENT expression { checkVarExists($1); }
  ;

arithmetical_assignment
  : IDENTIFIER ASSIGNMENT expression { checkVarExists($1); }
  ;

expression
  : OPEN_PARENTHESIS expression CLOSE_PARENTHESIS { $$ = $2; }
  | value { $$ = strdup("dummyValue"); }
  | function_call
  | IDENTIFIER array_index { checkVarExists($1); }
  | OPEN_PARENTHESIS type CLOSE_PARENTHESIS expression { $$ = $4; }
  | IDENTIFIER { checkVarExists($1); }
  | AMPERSAND IDENTIFIER { checkVarExists($2); }
  | ASTERISK IDENTIFIER { checkVarExists($2); }
  | expression ADDITION expression
  | expression SUBTRACTION expression 
  | expression DIVISION expression 
  | expression ASTERISK expression 
  | expression MODULUS expression
  | IDENTIFIER INCREMENT { checkVarExists($1); }
  | IDENTIFIER array_index INCREMENT { checkVarExists($1); }
  | IDENTIFIER DECREMENT { checkVarExists($1); }
  | IDENTIFIER array_index DECREMENT { checkVarExists($1); }
  | INCREMENT IDENTIFIER { checkVarExists($2); }
  | INCREMENT IDENTIFIER array_index { checkVarExists($2); }
  | DECREMENT IDENTIFIER { checkVarExists($2); }
  | DECREMENT IDENTIFIER array_index { checkVarExists($2); }
  ;

function_call
  : IDENTIFIER OPEN_PARENTHESIS arguments CLOSE_PARENTHESIS { checkFunExists($1); qCallFunction($1, $3); }
  | IDENTIFIER OPEN_PARENTHESIS CLOSE_PARENTHESIS { checkFunExists($1); qCallFunctionNoArgs($1); }
  | MALLOC OPEN_PARENTHESIS expression CLOSE_PARENTHESIS { qMalloc($3); }
  | SIZEOF OPEN_PARENTHESIS type CLOSE_PARENTHESIS { qSizeOf($3); }
  | PRINTF OPEN_PARENTHESIS expression CLOSE_PARENTHESIS { qPrint($3); }
  | PRINTF OPEN_PARENTHESIS STRING_VALUE COMMA arguments CLOSE_PARENTHESIS { qPrintExplicitFormat($3, $5);}
  | PRINTF OPEN_PARENTHESIS IDENTIFIER COMMA arguments CLOSE_PARENTHESIS { qPrintImplicitFormat($3, $5); }
  ;

arguments
  : arguments COMMA expression { char* pointer = malloc(200 * sizeof(char)); strcat(pointer, $1); strcat(pointer, ","); strcat(pointer, $3); $$ = pointer;}
  | expression { char* pointer = strdup($1); $$ = pointer;}
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
  : function_header { dummyReg(); } instruction_block
  ;

function_header
  : type function_subheader { functionDeclaration($1, $2, yylineno); }
  | VOID function_subheader { functionDeclaration(strdup("void"), $2, yylineno); }
  ;

function_subheader
  : IDENTIFIER OPEN_PARENTHESIS parameters CLOSE_PARENTHESIS { char* pointer = malloc(400 * sizeof(char)); strcat(pointer, $1); strcat(pointer, "("); strcat(pointer, $3); $$ = pointer; }
  | IDENTIFIER OPEN_PARENTHESIS CLOSE_PARENTHESIS { char* pointer = malloc(400 * sizeof(char)); strcat(pointer, $1); strcat(pointer, "("); $$ = pointer; }
  ;

parameters
  : type IDENTIFIER { char* pointer = malloc(50 * sizeof(char)); strcat(pointer, $1); strcat(pointer, " "); strcat(pointer, $2); $$ = pointer; }
  | parameters COMMA type IDENTIFIER { char* pointer = malloc(200 * sizeof(char)); strcat(pointer, $1); strcat(pointer, ","); strcat(pointer, $3); strcat(pointer, " "), strcat(pointer, $4); $$ = pointer;}
  ;

array_declaration
  : type IDENTIFIER array_index { declaration($1, $2, yylineno); }
  | type IDENTIFIER OPEN_SQUARE CLOSE_SQUARE ASSIGNMENT array { declaration($1, $2, yylineno); }
  ;

array_index
  : OPEN_SQUARE expression CLOSE_SQUARE
  ;

type
  : type ASTERISK { char* pointer = malloc(8*sizeof(char)); pointer = strdup($1); strcat(pointer, "*"); newReg(pointer, type, NULL, yylineno); $$ = pointer; }
  | INT { $$ = "int";}
  | CHAR { $$ = "char";}
  | FLOAT { $$ = "float";}
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
  if (argc>1) setObjFile(argv[2]);
  initST();
  dump("Initial ST");
  yyparse();
  dump("Final ST");
  clear();
  dump("ST Clean?");
}

void yyerror(char* message) {
  printf("Error in line %i: %s \n", yylineno, message);
  exit(-1);
}
