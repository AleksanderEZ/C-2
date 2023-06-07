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
int yylex();

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
  printf("Opening block %d\n", yylineno);
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

  char* tokens[10];
  char* token = strtok(NULL, ",");
  if (token != NULL) {
    char* definitiveToken = strdup(token);
    int i = 0;
    while (token != NULL) {
      definitiveToken = strdup(token);
      tokens[i] = definitiveToken;
      i++;
      token = strtok(NULL, ",");
    }

    newFunction(definitiveFunctionName, function, t, line, i);

    variableSwitch = localVariable;
    char* typeName;
    char* definitiveType;
    char* parameterName;
    char* definitiveParameterName;
    char* types[i];
    char* names[i];
    for(int j = 0; j < i; j++){
      typeName = strtok(tokens[j], " ");
      parameterName = strtok(NULL, " ");
      definitiveType = strdup(typeName);
      definitiveParameterName = strdup(parameterName);
      struct Reg* paramType = searchRegType(definitiveType, type);
      newParameter(definitiveParameterName, paramType, line);
      free(tokens[j]);
      types[j] = definitiveType;
      names[j] = definitiveParameterName;
    }
    struct Reg* functionResult = searchRegType(definitiveFunctionName, function);
    if (strcmp(definitiveFunctionName, "main") == 0) {
      qMain();
      functionResult->value = 0;
    } else {
      functionResult->value = qFunctionDeclaration(definitiveFunctionName, i, types, names);
    }
  } else {
    newFunction(definitiveFunctionName, function, t, line, 0);
    struct Reg* functionResult = searchRegType(definitiveFunctionName, function);
    if (strcmp(definitiveFunctionName, "main") == 0) {
      qMain();
      functionResult->value = 0;
    } else {
      functionResult->value = qFunctionDeclaration(definitiveFunctionName, 0, NULL, NULL);
    }
  }
  free(name);
}

void checkFunExists(char* name) {
  if (searchRegType(name, function) == NULL) yyerror("Function has not been declared");
}

void checkVarExists(char* name) {
  if (searchRegType(name, globalVariable) == NULL && searchRegType(name, localVariable) == NULL && searchRegType(name, parameter) == NULL) yyerror("Variable has not been declared");
}

%}

%union { float real; int integer; char character; char* string; }
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
%token ADDITION SUBTRACTION DIVISION MODULUS EQUALS NOT_EQUALS GREATER GREATER_EQUALS LESSER LESSER_EQUALS AND OR NEGATOR ASTERISK AMPERSAND
%token COMMA SEMICOLON ASSIGNMENT OPEN_CURLY CLOSE_CURLY OPEN_PARENTHESIS CLOSE_PARENTHESIS OPEN_SQUARE CLOSE_SQUARE
%token WHILE FOR BREAK CONTINUE RETURN IF ELSE ERROR

%type <string> type
%type <string> function_subheader
%type <string> function_header
%type <string> parameters

%type <integer> condition
%type <integer> expression
%type <integer> value
%type <integer> function_call
%type <integer> array_index
%type <integer> value_list

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
  : simple_instruction SEMICOLON
  | complex_instruction
  ;

simple_instruction
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
  | type IDENTIFIER ASSIGNMENT expression { declaration($1, $2, yylineno); qStoreVar($4, $2); qFreeRegister($4); }
  | type IDENTIFIER OPEN_SQUARE INT_VALUE CLOSE_SQUARE { declaration($1, $2, yylineno); qReserveMemory($1, $2, $4, variableSwitch); }
  | type IDENTIFIER OPEN_SQUARE CLOSE_SQUARE ASSIGNMENT OPEN_CURLY { qNewValueList($1, variableSwitch); } value_list CLOSE_CURLY { declaration($1, $2, yylineno); qReserveArray($2); }
  ;

complex_instruction
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
  | IDENTIFIER ASSIGNMENT expression
  | type IDENTIFIER ASSIGNMENT expression { variableSwitch = localVariable; declaration($1, $2, yylineno); variableSwitch = globalVariable; }
  ;

third_part_for
  : 
  | assignment
  ;

while
  : while_header { dummyReg(); } instruction { qFinishWhile(); }
  ;

while_header
  : WHILE OPEN_PARENTHESIS { qStartWhile(); } condition { qWhileCondition($4); qFreeRegister($4); } CLOSE_PARENTHESIS 
  ;

if
  : if_header simple_instruction SEMICOLON { qElse(); closeBlock(); }
  | if_header simple_instruction SEMICOLON { qSkipElse(); closeBlock(); } else
  | if_header instruction_block { qElse(); }
  | if_header instruction_block { qSkipElse(); } else
  ;

if_header
  : IF { dummyReg(); } OPEN_PARENTHESIS condition { qIf($4); qFreeRegister($4); } CLOSE_PARENTHESIS
  ;

else
  : ELSE { dummyReg(); qElse(); } instruction { qSkipElseLabel(); }
  ;

condition
  : OPEN_PARENTHESIS condition CLOSE_PARENTHESIS { $$ = $2; }
  | condition AND condition { $$ = $1; qAnd($1, $3); qFreeRegister($3); }
  | condition OR condition { $$ = $1; qAnd($1, $3); qFreeRegister($3); }
  | NEGATOR condition { $$ = $2; qNegate($2); qFreeRegister($2); }
  | '1' { $$ = qAssignRegister(); qLoadIntValue($$, 1); }
  | '0' { $$ = qAssignRegister(); qLoadIntValue($$, 0); }
  | expression EQUALS expression { $$ = $1; qEquals($1, $3); qFreeRegister($3); }
  | expression NOT_EQUALS expression { $$ = $1; qNotEquals($1, $3); qFreeRegister($3); }
  | expression GREATER expression { $$ = $1; qGreater($1, $3); qFreeRegister($3); }
  | expression GREATER_EQUALS expression { $$ = $1; qGreaterEquals($1, $3); qFreeRegister($3); }
  | expression LESSER expression { $$ = $1; qLesser($1, $3); qFreeRegister($3); }
  | expression LESSER_EQUALS expression { $$ = $1; qLesserEquals($1, $3); qFreeRegister($3); }
  ;

assignment
  : IDENTIFIER ASSIGNMENT expression { checkVarExists($1); qStoreVar($3, $1); qFreeRegister($3); }
  | IDENTIFIER array_index ASSIGNMENT expression { checkVarExists($1); qStoreArrayIndex($1, $2, $4); qFreeRegister($4); }
  ;

expression
  : OPEN_PARENTHESIS expression CLOSE_PARENTHESIS { $$ = $2; }
  | value
  | IDENTIFIER { checkVarExists($1); $$ = qAssignRegister(); qLoadVar($$, $1); }
  | expression ADDITION expression { $$ = $1; qAdd($1, $3); qFreeRegister($3);}
  | expression SUBTRACTION expression { $$ = $1; qSubtract($1, $3); qFreeRegister($3);}
  | expression DIVISION expression { $$ = $1; qDivide($1, $3); qFreeRegister($3);}
  | expression ASTERISK expression { $$ = $1; qMultiply($1, $3); qFreeRegister($3);}
  | expression MODULUS expression { $$ = $1; qModulus($1, $3); qFreeRegister($3);}
  | function_call
  | IDENTIFIER array_index { checkVarExists($1); $$ = qAssignRegister(); qArrayAccess($$, $1, $2); qFreeRegister($2); }
//  | ASTERISK IDENTIFIER { checkVarExists($2); }
//  | AMPERSAND IDENTIFIER { checkVarExists($2); }
//  | OPEN_PARENTHESIS type CLOSE_PARENTHESIS expression { $$ = $4; }
  ;

function_call
  : IDENTIFIER OPEN_PARENTHESIS arguments CLOSE_PARENTHESIS { checkFunExists($1); dummyReg(); $$ = qCallFunction($1); closeBlock(); }
  | IDENTIFIER OPEN_PARENTHESIS CLOSE_PARENTHESIS { checkFunExists($1); $$ = qCallFunctionNoArgs($1); }
//  | MALLOC OPEN_PARENTHESIS expression CLOSE_PARENTHESIS { qMalloc($3); }
//  | SIZEOF OPEN_PARENTHESIS type CLOSE_PARENTHESIS { qSizeOf($3); }
  | PRINTF OPEN_PARENTHESIS STRING_VALUE COMMA expression CLOSE_PARENTHESIS { qPrintExplicitFormat($3, $5); qFreeRegister($5);}
  | PRINTF OPEN_PARENTHESIS STRING_VALUE CLOSE_PARENTHESIS { qPrintExplicit($3); }
  | PRINTF OPEN_PARENTHESIS IDENTIFIER COMMA expression CLOSE_PARENTHESIS { qPrintImplicitFormat($3, $5); qFreeRegister($5);}
  ;

arguments
  : arguments COMMA expression { qFunctionArguments($3); }
  | expression { qFunctionArguments($1); };

return
  : RETURN expression { qReturn($2); }
  | RETURN { qReturn(400); }
  ;

value_list
  : expression  { qExpandValueList($1); qFreeRegister($1); }
  | value_list COMMA expression { qExpandValueList($3); qFreeRegister($1); }
  ;

array_index
  : OPEN_SQUARE expression CLOSE_SQUARE { $$ = $2; }
  ;

function_declaration
  : function_header { dummyReg(); } instruction_block { if(strncmp($1, "main", 4) != 0) qFinishFunction(); }
  ;

function_header
  : type function_subheader { functionDeclaration($1, $2, yylineno); $$ = $2; }
  | VOID function_subheader { functionDeclaration(strdup("void"), $2, yylineno); $$ = $2; }
  ;

function_subheader
  : IDENTIFIER OPEN_PARENTHESIS parameters CLOSE_PARENTHESIS { char* pointer = calloc(1, 400 * sizeof(char)); strcat(pointer, $1); free($1); strcat(pointer, "("); strcat(pointer, $3); free($3); $$ = pointer; }
  | IDENTIFIER OPEN_PARENTHESIS CLOSE_PARENTHESIS { char* pointer = calloc(1, 400 * sizeof(char)); strcat(pointer, $1); free($1); strcat(pointer, "("); $$ = pointer; }
  ;

parameters
  : type IDENTIFIER { char* pointer = calloc(1, 50 * sizeof(char)); strcat(pointer, $1); strcat(pointer, " "); strcat(pointer, $2); free($2); $$ = pointer; }
  | parameters COMMA type IDENTIFIER { char* pointer = calloc(1, 200 * sizeof(char)); strcat(pointer, $1); free($1); strcat(pointer, ","); strcat(pointer, $3); strcat(pointer, " "), strcat(pointer, $4); free($4); $$ = pointer;}
  ;

type
  : type ASTERISK { char* pointer = calloc(1, 8*sizeof(char)); pointer = strdup($1); strcat(pointer, "*"); newReg(pointer, type, NULL, yylineno); $$ = pointer; }
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
