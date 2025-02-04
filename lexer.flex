%top{
    #include "parser.tab.hh"
    #define YY_DECL yy::parser::symbol_type yylex()
    #include "Node.h"
    int lexical_errors = 0;
}
%option yylineno noyywrap nounput batch noinput stack
%%

"public"                {return yy::parser::make_PUBLIC(yytext);}
"class"                 {return yy::parser::make_CLASS(yytext);}
"static"                {return yy::parser::make_STATIC(yytext);}
"void"                  {return yy::parser::make_VOID(yytext);}
"main"                  {return yy::parser::make_MAIN(yytext);}
"String"                {return yy::parser::make_STRING(yytext);}
"return"                {return yy::parser::make_RETURN(yytext);}
"int[]"                 {return yy::parser::make_INTARR(yytext);}
"boolean"               {return yy::parser::make_BOOL(yytext);}
"int"                   {return yy::parser::make_INT(yytext);}
"if"                    {return yy::parser::make_IF(yytext);}
"else"                  {return yy::parser::make_ELSE(yytext);}
"while"                 {return yy::parser::make_WHILE(yytext);}
"System.out.println"    {return yy::parser::make_PRINT(yytext);}

"="                     {return yy::parser::make_EQUALSSIGN(yytext);}
"("                     {return yy::parser::make_LP(yytext);}
")"                     {return yy::parser::make_RP(yytext);}
"}"                     {return yy::parser::make_RC(yytext);}
"{"                     {return yy::parser::make_LC(yytext);}
";"                     {return yy::parser::make_SEMCOL(yytext);}
","                     {return yy::parser::make_COMMA(yytext);}

"&&"                    {return yy::parser::make_ANDEXPR(yytext);}
"||"                    {return yy::parser::make_OREXPR(yytext);}
"<"                     {return yy::parser::make_LTEXPR(yytext);}
">"                     {return yy::parser::make_GTEXPR(yytext);}
"=="                    {return yy::parser::make_EQUALSEXPR(yytext);}
"+"                     {return yy::parser::make_PLUSOP(yytext);}
"-"                     {return yy::parser::make_MINUSOP(yytext);}
"*"                     {return yy::parser::make_MULTOP(yytext);}

"["                     {return yy::parser::make_LB(yytext);}
"]"                     {return yy::parser::make_RB(yytext);}
"."                     {return yy::parser::make_DOT(yytext);}
"length"                {return yy::parser::make_LEN(yytext);}
"this"                  {return yy::parser::make_THIS(yytext);}
"new"                   {return yy::parser::make_NEW(yytext);}
"!"                     {return yy::parser::make_EXCLMARK(yytext);}

"true"                  {return yy::parser::make_TRUE(yytext);}
"false"                 {return yy::parser::make_FALSE(yytext);}
0|[1-9][0-9]*           {return yy::parser::make_INTLIT(yytext);}
[A-Za-z_]+[0-9A-Za-z_]* {return yy::parser::make_STRLIT(yytext);}

[ \t\n\r]+              {}
"//"[^\n]*              {}

.                       {if(!lexical_errors) fprintf(stderr, "Lexical errors found! See the logs below: \n"); fprintf(stderr,"\t@error at line %d. Character %s is not recognized\n", yylineno, yytext); lexical_errors = 1;}

<<EOF>>                 {return yy::parser::make_END();}
%%
