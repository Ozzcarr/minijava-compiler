%top{
    #include "parser.tab.hh"
    #define YY_DECL yy::parser::symbol_type yylex()
    #include "Node.h"
    int lexical_errors = 0;
}
%option yylineno noyywrap nounput batch noinput stack 
%%
"public"                {if(USE_LEX_ONLY) {printf("PUBLIC ");}}
"class"                 {if(USE_LEX_ONLY) {printf("CLASS ");}}
"{"                     {if(USE_LEX_ONLY) {printf("LC ");}}                    
"static"                {if(USE_LEX_ONLY) {printf("STATIC ");}}
"void"                  {if(USE_LEX_ONLY) {printf("VOID ");}}
"main"                  {if(USE_LEX_ONLY) {printf("MAIN ");}}
"("                     {if(USE_LEX_ONLY) {printf("LP ");} else {return yy::parser::make_LP(yytext);}}
"String"                {if(USE_LEX_ONLY) {printf("STRING ");}}
"["                     {if(USE_LEX_ONLY) {printf("LB ");}}
"]"                     {if(USE_LEX_ONLY) {printf("RB ");}}
")"                     {if(USE_LEX_ONLY) {printf("RP ");} else {return yy::parser::make_RP(yytext);}}
"}"                     {if(USE_LEX_ONLY) {printf("RC ");}}
";"                     {if(USE_LEX_ONLY) {printf("SEMCOL ");}}
","                     {if(USE_LEX_ONLY) {printf("COMMA ");}}
"return"                {if(USE_LEX_ONLY) {printf("RETURN ");}}
"int[]"                 {if(USE_LEX_ONLY) {printf("INTARR ");}}
"boolean"               {if(USE_LEX_ONLY) {printf("BOOL ");}}
"int"                   {if(USE_LEX_ONLY) {printf("INT ");}}
"if"                    {if(USE_LEX_ONLY) {printf("IF ");}}
"else"                  {if(USE_LEX_ONLY) {printf("ELSE ");}}
"while"                 {if(USE_LEX_ONLY) {printf("WHILE ");}}
"System.out.println"    {if(USE_LEX_ONLY) {printf("PRINT ");}}
"="                     {if(USE_LEX_ONLY) {printf("EQUALSSIGN ");}}
"&&"                    {if(USE_LEX_ONLY) {printf("ANDEXPR ");}}
"||"                    {if(USE_LEX_ONLY) {printf("OREXPR ");}}
"<"                     {if(USE_LEX_ONLY) {printf("LTEXPR ");}}
">"                     {if(USE_LEX_ONLY) {printf("GREXPR ");}}
"=="                    {if(USE_LEX_ONLY) {printf("EQUALSEXPR ");}}
"+"                     {if(USE_LEX_ONLY) {printf("PLUSOP ");} else {return yy::parser::make_PLUSOP(yytext);}}
"-"                     {if(USE_LEX_ONLY) {printf("SUBOP ");} else {return yy::parser::make_MINUSOP(yytext);}}
"*"                     {if(USE_LEX_ONLY) {printf("MULTOP ");} else {return yy::parser::make_MULTOP(yytext);}}
"."                     {if(USE_LEX_ONLY) {printf("DOT ");}}
"length"                {if(USE_LEX_ONLY) {printf("LEN ");}}
"true"                  {if(USE_LEX_ONLY) {printf("TRUE ");}}
"false"                 {if(USE_LEX_ONLY) {printf("FALSE ");}}
"this"                  {if(USE_LEX_ONLY) {printf("THIS ");}}
"new"                   {if(USE_LEX_ONLY) {printf("NEW ");}}
"!"                     {if(USE_LEX_ONLY) {printf("EXCLMARK ");}}
"_"                     {if(USE_LEX_ONLY) {printf("UNDRSCR ");}}


0|[1-9][0-9]*           {if(USE_LEX_ONLY) {printf("INTLIT ");} else {return yy::parser::make_INT(yytext);}}
[A-Za-z]*               {if(USE_LEX_ONLY) {printf("-- ");}}

[ \t\n\r]+              {}
"//"[^\n]*              {}
.                       { if(!lexical_errors) fprintf(stderr, "Lexical errors found! See the logs below: \n"); fprintf(stderr,"\t@error at line %d. Character %s is not recognized\n", yylineno, yytext); lexical_errors = 1;}
<<EOF>>                  {return yy::parser::make_END();}
%%