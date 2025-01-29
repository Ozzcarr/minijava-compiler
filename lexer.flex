%top{
    #include "parser.tab.hh"
    #define YY_DECL yy::parser::symbol_type yylex()
    #include "Node.h"
    int lexical_errors = 0;
}
%option yylineno noyywrap nounput batch noinput stack 
%%

"public"                {if(USE_LEX_ONLY) {printf("PUBLIC ");} else {return yy::parser::make_PUBLIC(yytext);}}
"class"                 {if(USE_LEX_ONLY) {printf("CLASS ");} else {return yy::parser::make_CLASS(yytext);}}
"static"                {if(USE_LEX_ONLY) {printf("STATIC ");} else {return yy::parser::make_STATIC(yytext);}}
"void"                  {if(USE_LEX_ONLY) {printf("VOID ");} else {return yy::parser::make_VOID(yytext);}}
"main"                  {if(USE_LEX_ONLY) {printf("MAIN ");} else {return yy::parser::make_MAIN(yytext);}}
"String"                {if(USE_LEX_ONLY) {printf("STRING ");} else {return yy::parser::make_STRING(yytext);}}
"return"                {if(USE_LEX_ONLY) {printf("RETURN ");} else {return yy::parser::make_RETURN(yytext);}}
"int[]"                 {if(USE_LEX_ONLY) {printf("INTARR ");} else {return yy::parser::make_INTARR(yytext);}}
"boolean"               {if(USE_LEX_ONLY) {printf("BOOL ");} else {return yy::parser::make_BOOL(yytext);}}
"int"                   {if(USE_LEX_ONLY) {printf("INT ");} else {return yy::parser::make_INT(yytext);}}
"if"                    {if(USE_LEX_ONLY) {printf("IF ");} else {return yy::parser::make_IF(yytext);}}
"else"                  {if(USE_LEX_ONLY) {printf("ELSE ");} else {return yy::parser::make_ELSE(yytext);}}
"while"                 {if(USE_LEX_ONLY) {printf("WHILE ");} else {return yy::parser::make_WHILE(yytext);}}
"System.out.println"    {if(USE_LEX_ONLY) {printf("PRINT ");} else {return yy::parser::make_PRINT(yytext);}}

"="                     {if(USE_LEX_ONLY) {printf("EQUALSSIGN ");} else {return yy::parser::make_EQUALSSIGN(yytext);}}
"("                     {if(USE_LEX_ONLY) {printf("LP ");} else {return yy::parser::make_LP(yytext);}}
")"                     {if(USE_LEX_ONLY) {printf("RP ");} else {return yy::parser::make_RP(yytext);}}
"}"                     {if(USE_LEX_ONLY) {printf("RC ");} else {return yy::parser::make_RC(yytext);}}
"{"                     {if(USE_LEX_ONLY) {printf("LC ");} else {return yy::parser::make_LC(yytext);}}
";"                     {if(USE_LEX_ONLY) {printf("SEMCOL ");} else {return yy::parser::make_SEMCOL(yytext);}}
","                     {if(USE_LEX_ONLY) {printf("COMMA ");} else {return yy::parser::make_COMMA(yytext);}}

"&&"                    {if(USE_LEX_ONLY) {printf("ANDEXPR ");} else {return yy::parser::make_ANDEXPR(yytext);}}
"||"                    {if(USE_LEX_ONLY) {printf("OREXPR ");} else {return yy::parser::make_OREXPR(yytext);}}
"<"                     {if(USE_LEX_ONLY) {printf("LTEXPR ");} else {return yy::parser::make_LTEXPR(yytext);}}
">"                     {if(USE_LEX_ONLY) {printf("GTEXPR ");} else {return yy::parser::make_GTEXPR(yytext);}}
"=="                    {if(USE_LEX_ONLY) {printf("EQUALSEXPR ");} else {return yy::parser::make_EQUALSEXPR(yytext);}}
"+"                     {if(USE_LEX_ONLY) {printf("PLUSOP ");} else {return yy::parser::make_PLUSOP(yytext);}}
"-"                     {if(USE_LEX_ONLY) {printf("MINUSOP ");} else {return yy::parser::make_MINUSOP(yytext);}}
"*"                     {if(USE_LEX_ONLY) {printf("MULTOP ");} else {return yy::parser::make_MULTOP(yytext);}}
"_"                     {if(USE_LEX_ONLY) {printf("UNDRSCR ");} else {return yy::parser::make_UNDRSCR(yytext);}}

"["                     {if(USE_LEX_ONLY) {printf("LB ");} else {return yy::parser::make_LB(yytext);}}
"]"                     {if(USE_LEX_ONLY) {printf("RB ");} else {return yy::parser::make_RB(yytext);}}
"."                     {if(USE_LEX_ONLY) {printf("DOT ");} else {return yy::parser::make_DOT(yytext);}}
"length"                {if(USE_LEX_ONLY) {printf("LEN ");} else {return yy::parser::make_LEN(yytext);}}
"this"                  {if(USE_LEX_ONLY) {printf("THIS ");} else {return yy::parser::make_THIS(yytext);}}
"new"                   {if(USE_LEX_ONLY) {printf("NEW ");} else {return yy::parser::make_NEW(yytext);}}
"!"                     {if(USE_LEX_ONLY) {printf("EXCLMARK ");} else {return yy::parser::make_EXCLMARK(yytext);}}

"true"                  {if(USE_LEX_ONLY) {printf("TRUE ");} else {return yy::parser::make_TRUE(yytext);}}
"false"                 {if(USE_LEX_ONLY) {printf("FALSE ");} else {return yy::parser::make_FALSE(yytext);}}
0|[1-9][0-9]*           {if(USE_LEX_ONLY) {printf("INTLIT ");} else {return yy::parser::make_INTLIT(yytext);}}
[A-Za-z]+[0-9A-Za-z]*   {if(USE_LEX_ONLY) {printf("STRLIT ");} else {return yy::parser::make_STRLIT(yytext);}}

[ \t\n\r]+              {if(USE_LEX_ONLY) {printf(yytext);}}
"//"[^\n]*              {}

.                       {if(!lexical_errors) fprintf(stderr, "Lexical errors found! See the logs below: \n"); fprintf(stderr,"\t@error at line %d. Character %s is not recognized\n", yylineno, yytext); lexical_errors = 1;}

<<EOF>>                  {return yy::parser::make_END();}
%%
