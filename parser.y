%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symbol_table.h"
#include "code_generation.h"
#include "Qlib.h"

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

void checkIsInt(char* name) {
  struct Reg* variable = searchRegType("int", name);
  if (variable == NULL) yyerror("Variable is not of type int");
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

%type <integer> expression
%type <integer> value
%type <integer> function_call
%type <integer> condition

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
  : while_header instruction { qFinishWhile(); }
  ;

while_header
  : WHILE { dummyReg(); } OPEN_PARENTHESIS { qStartWhile(); } condition CLOSE_PARENTHESIS
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
  : OPEN_PARENTHESIS condition CLOSE_PARENTHESIS { $$ = $2; }
  | condition AND condition { $$ = $1; qAnd($1, $3); qFreeRegister($$); }
  | condition OR condition { $$ = $1; qAnd($1, $3); qFreeRegister($$); }
  | NEGATOR condition { $$ = $2; qNegate($2); qFreeRegister($$); }
  | '1' { $$ = qAssignRegister(); qLoadIntValue($$, 1); }
  | '0' { $$ = qAssignRegister(); qLoadIntValue($$, 0); }
  | expression EQUALS expression { $$ = $1; qEquals($1, $3); qFreeRegister($$); }
  | expression NOT_EQUALS expression { $$ = $1; qNotEquals($1, $3); qFreeRegister($$); }
  | expression GREATER expression { $$ = $1; qGreater($1, $3); qFreeRegister($$); }
  | expression GREATER_EQUALS expression { $$ = $1; qGreaterEquals($1, $3); qFreeRegister($$); }
  | expression LESSER expression { $$ = $1; qLesser($1, $3); qFreeRegister($$); }
  | expression LESSER_EQUALS expression { $$ = $1; qLesserEquals($1, $3); qFreeRegister($$); }
  ;

assignment
  : arithmetical_assignment
  | IDENTIFIER array_index ASSIGNMENT expression { checkVarExists($1); setRegValue(); }
  ;

arithmetical_assignment
  : IDENTIFIER ASSIGNMENT expression { checkVarExists($1); }
  ;

expression
  : OPEN_PARENTHESIS expression CLOSE_PARENTHESIS { $$ = $2; }
  | value
  | IDENTIFIER { checkVarExists($1); $$ = qAssignRegister(); qLoadVar($$, $1, variableSwitch); }
  | expression ADDITION expression { $$ = $1; qAdd($1, $3); qFreeRegister($$);}
  | expression SUBTRACTION expression { $$ = $1; qSubtract($1, $3); qFreeRegister($$);}
  | expression DIVISION expression { $$ = $1; qDivide($1, $3); qFreeRegister($$);}
  | expression ASTERISK expression { $$ = $1; qMultiply($1, $3); qFreeRegister($$);}
  | expression MODULUS expression { $$ = $1; qModulus($1, $3); qFreeRegister($$);}
  | function_call
  | IDENTIFIER array_index { checkVarExists($1); }
  | AMPERSAND IDENTIFIER { checkVarExists($2); }
  | ASTERISK IDENTIFIER { checkVarExists($2); }
  | OPEN_PARENTHESIS type CLOSE_PARENTHESIS expression { $$ = $4; }
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
  : RETURN expression { $$ = qReturn($2); }
  | RETURN
  ;

// array

array
  : OPEN_CURLY value_list CLOSE_CURLY { $$ = $2 }
  ;

value_list
  : expression
  | value_list COMMA expression
  ;

array_declaration
  : type IDENTIFIER array_index { declaration($1, $2, yylineno); void* address = qReserveMemory($3); setRegValue($2, variableSwitch, address); }
  | type IDENTIFIER OPEN_SQUARE CLOSE_SQUARE ASSIGNMENT array { declaration($1, $2, yylineno); void* address = qReserveArray($6, arraySize, arrayType); setRegValue($2, variableSwitch, address); arraySize = 0; arrayType = voidType;}
  ;

array_index
  : OPEN_SQUARE expression CLOSE_SQUARE { $$ = $2 }
  ;

//

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

type
  : type ASTERISK { char* pointer = malloc(8*sizeof(char)); pointer = strdup($1); strcat(pointer, "*"); newReg(pointer, type, NULL, yylineno); $$ = pointer; }
  | INT { $$ = "int";}
  | CHAR { $$ = "char";}
  | FLOAT { $$ = "float";}
  ;

value
  : INT_VALUE { $$ = qAssignRegister(); qLoadIntValue($$, $1); }
  | FLOAT_VALUE { $$ = qAssignRegister(); qLoadFloatValue($$, $1); }
  | CHAR_VALUE { $$ = qAssignRegister(); qLoadCharValue($$, $1); }
  | STRING_VALUE { $$ = qAssignRegister(); qLoadStringValue($$, $1); }
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
