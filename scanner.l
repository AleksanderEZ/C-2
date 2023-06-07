%{
#include "parser.tab.h"
#include <stdio.h>
#include <string.h>
%}
%option yylineno

integer -?[0-9]+
floating {integer}\.[0-9]*
comment (\/\/.*\n)|(\/\*(.|\n)*\*\/)
nombre [a-zA-Z_]([a-zA-Z]|[0-9])*
character '([^\r\n'])'
string \"[^\"\n]*\"

%%
[ \n\t]                   
{comment}               
,                       return COMMA;
;                       return SEMICOLON;
=                       return ASSIGNMENT;
\{                      return OPEN_CURLY;
\}                      return CLOSE_CURLY;
\(                      return OPEN_PARENTHESIS;
\)                      return CLOSE_PARENTHESIS;
\[                      return OPEN_SQUARE;
\]                      return CLOSE_SQUARE;
int                     return INT;
char                    return CHAR;
float                   return FLOAT;
void                    return VOID;
while                   return WHILE;
for                     return FOR;
break                   return BREAK;
continue                return CONTINUE;
return                  return RETURN;
if                      return IF;
else                    return ELSE;
sizeof                  return SIZEOF;
malloc                  return MALLOC;
printf                  return PRINTF;
\+                      return ADDITION;
-                       return SUBTRACTION;
\/                      return DIVISION;
\*                      return ASTERISK;
&                       return AMPERSAND;
\%                      return MODULUS;
==                      return EQUALS;
!=                      return NOT_EQUALS;
>                       return GREATER;
\<                      return LESSER;
>=                      return GREATER_EQUALS;
\<=                     return LESSER_EQUALS;
&&                      return AND;
\|\|                    return OR;
!                       return NEGATOR;
{nombre}                { yylval.string = strdup(yytext); return IDENTIFIER; }
{character}             { yylval.character = strdup(yytext)[1]; return CHAR_VALUE;}
{string}                { yylval.string = strdup(yytext); return STRING_VALUE; }
{integer}               { yylval.integer = atoi(yytext); return INT_VALUE; }
{floating}              { yylval.real = atof(yytext); return FLOAT_VALUE; }
.                       return ERROR;
%%

int yywrap() {
    printf("Read lines: %d\n", yylineno - 1);
    return 1;
}