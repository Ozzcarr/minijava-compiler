/* Skeleton and definitions for generating a LALR(1) parser in C++ */
%skeleton "lalr1.cc" 
%defines
%define parse.error verbose
%define api.value.type variant
%define api.token.constructor

/* Required code included before the parser definition begins */
%code requires{
  #include <string>
  #include "Node.h"
  #define USE_LEX_ONLY false //change this macro to true if you want to isolate the lexer from the parser.
}

/* Code included in the parser implementation file */
%code{
  #define YY_DECL yy::parser::symbol_type yylex()
  YY_DECL;

  Node* root;
  extern int yylineno;
}

/* Token definitions for the grammar */
/* Tokens represent the smallest units of the language, like operators and parentheses */
%token <std::string> PUBLIC CLASS STATIC VOID MAIN STRING RETURN INTARR BOOL INT IF ELSE WHILE PRINT EQUALSSIGN LP RP RC LC SEMCOL COMMA ANDEXPR OREXPR LTEXPR GTEXPR EQUALSEXPR PLUSOP MINUSOP SUBOP MULTOP UNDRSCR LB RB DOT LEN THIS NEW EXCLMARK TRUE FALSE INTLIT STRLIT
%token END 0 "end of file"

/* Operator precedence and associativity rules */
%left OREXPR
%left ANDEXPR
%left EQUALSEXPR
%left LTEXPR GTEXPR
%left PLUSOP MINUSOP
%left MULTOP
%left LP RP
%right EXCLMARK

/* Specify types for non-terminals in the grammar */
/* The type specifies the data type of the values associated with these non-terminals */
%type <Node *> root statements condition iftruecondition elsecondition statement else expression argument_list identifier

/* Grammar rules section */
/* This section defines the production rules for the language being parsed */
%%
root:       statement {root = $1;};

statements:
    %empty
    {
        // Empty list of statements
        $$ = new Node("Statements", "", yylineno);  // Create an empty "Statements" node
    }
    | statements statement
    {
        // Append the new statement to the existing list
        $1->children.push_back($2);
        $$ = $1;
    }
    ;

condition: expression {
                        $$ = new Node("Condition", "", yylineno);
                        $$->children.push_back($1);
}
;

iftruecondition: statement {
                        $$ = new Node("IfTrue", "", yylineno);
                        $$->children.push_back($1);
}
;

elsecondition: else {
                        $$ = new Node("Else", "", yylineno);
                        $$->children.push_back($1);
}
;

statement: LC statements RC {
                            $$ = $2;
                          }
            | IF LP condition RP iftruecondition elsecondition {
                            $$ = new Node("IfStatement", "", yylineno);
                            $$->children.push_back($3);
                            $$->children.push_back($5);
                            $$->children.push_back($6);
                          }
            | WHILE LP expression RP statement {
                            $$ = new Node("WhileStatement", "", yylineno);
                            $$->children.push_back($3);
                            $$->children.push_back($5);
                          }
            | PRINT LP expression RP SEMCOL{
                            $$ = $3;
                          }
            | identifier EQUALSSIGN expression SEMCOL {
                            $$ = new Node("VarInitStatement", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | identifier LB expression RB EQUALSSIGN expression SEMCOL {
                            $$ = new Node("ArrayInitStatement", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                            $$->children.push_back($6);
                          }
;

else:                     {
                            $$ = new Node("ElsePart", "", yylineno); 
                          }
            | ELSE statement {
                            $$ = $2;
                          }
;

expression:   expression ANDEXPR expression {
                            $$ = new Node("AndExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression OREXPR expression {
                            $$ = new Node("OrExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression LTEXPR expression {
                            $$ = new Node("LTExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression GTEXPR expression {
                            $$ = new Node("GTExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression EQUALSEXPR expression {
                            $$ = new Node("EqualExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression PLUSOP expression {
                            $$ = new Node("AddExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression MINUSOP expression {
                            $$ = new Node("SubExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression MULTOP expression {
                            $$ = new Node("MultExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression LB expression RB {
                            $$ = new Node("ArrayExpression", "", yylineno);
                            $$->children.push_back($1);
                            $$->children.push_back($3);
                          }
            | expression DOT LEN {
                            $$ = $1;
                          }
            | expression DOT identifier LP argument_list RP {
                            $$ = $1;
                          }
            | INTLIT {
                            $$ = new Node("IntLit", $1, yylineno); 
                          }
            | TRUE {
                            $$ = new Node("Bool", $1, yylineno);
                          }
            | FALSE {
                            $$ = new Node("Bool", $1, yylineno);
                          }
            | identifier {
                            $$ = $1;
                          }
            | THIS {
                            $$ = new Node("ThisExpression", $1, yylineno);
                          }
            | NEW INT LB expression RB {
                            $$ = $4;
                          }
            | NEW identifier LP RP {
                            $$ = $2;
                          }
            | EXCLMARK expression {
                            $$ = $2;
                          }
            | LP expression RP { 
                            $$ = $2; 
                          }
;

argument_list: {
                            $$ = new Node("ArgumentList", "", yylineno);
                            }
            | expression {
                            $$ = $1;
                            }
            | argument_list COMMA expression {
                            $$ = new Node("ArgumentList", "", yylineno);
                            $$->children.push_back($1);  // Add previous arguments
                            $$->children.push_back($3);  // Add current argument
                            }
;

identifier: STRLIT {
                          $$ = new Node("StringLit", $1, yylineno); 
                          }
            | INT {
                          $$ = new Node("INT", $1, yylineno); 
            }
;
