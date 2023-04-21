%{
#include "parser.tab.h"
#include <stdio.h>
%}
%option yylineno

integer [0-9]+
floating {integer}\.[0-9]*
comment (\/\/.*\n)|(\/\*(.|\n)*\*\/)
nombre [a-zA-z_]([a-zA-Z]|[0-9])*
character '([^\r\n'])'
string \"[^\"\n]*\"

%%
[ \n\t]                   
{comment}               return COMMENT;
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
\+\+                    return INCREMENT;
-                       return SUBTRACTION;
--                      return DECREMENT;
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
{nombre}                return IDENTIFIER;
{character}             return CHAR_VALUE;
{string}                return STRING_VALUE;
{integer}               return INT_VALUE;
{floating}              return FLOAT_VALUE;
.                       return ERROR;
%%

int yywrap() {
    printf("Read lines: %d\n", yylineno - 1);
    return 1;
}