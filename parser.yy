/* Skeleton and definitions for generating a LALR(1) parser in C++ */
%skeleton "lalr1.cc"
%defines
%define parse.error verbose
%define api.value.type variant
%define api.token.constructor

/* Required code included before the parser definition begins */
%code requires {
  #include <string>
  #include "Node.h"
  #define USE_LEX_ONLY false // change this macro to true if you want to isolate the lexer from the parser.
}

/* Code included in the parser implementation file */
%code {
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
%right EXCLMARK
%left LP RP LB RB DOT

/* Prevents shift/reduce conflicts for if else statements */
%nonassoc ELSE

/* Specify types for non-terminals in the grammar */
/* The type specifies the data type of the values associated with these non-terminals */
%type <Node *> root variable variable_list return code method type statements condition statement expression argument_list identifier next_row

/* Grammar rules section */
/* This section defines the production rules for the language being parsed */
%%
root:
  method { root = $1; };

variable:
  type STRLIT {
    $$ = new Node("Variable", $2, yylineno);
    $$->children.push_back($1);
  };

variable_list:
  %empty {
    $$ = new Node("VariableList", "", yylineno);
  }
  | variable {
    $$ = $1;
  }
  | variable_list COMMA variable {
    $$ = new Node("VariableList", "", yylineno);
    $$->children.push_back($1);
    $$->children.push_back($3);
  };

return:
  RETURN expression {
    $$ = new Node("Return", "", yylineno);
    $$->children.push_back($2);
  };

code:
  %empty {
    $$ = new Node("Code", "", yylineno);
  }
  | code next_row {
    $$ = $1;
    $$->children.push_back($2);
  };

next_row:
  variable {
    $$ = $1;
  }
  | statement {
    $$ = $1;
  };

method:
  PUBLIC type STRLIT LP variable_list RP LC code return SEMCOL RC {
    $$ = new Node("Method", $3, yylineno);
    $$->children.push_back($2);
    $$->children.push_back($5);
    $$->children.push_back($8);
    $$->children.push_back($9);
  };

type:
  INT LB RB {
    $$ = new Node("Type", " IntArray", yylineno);
  }
  | BOOL {
    $$ = new Node("Type", " Bool", yylineno);
  }
  | INT {
    $$ = new Node("Type", " Int", yylineno);
  }
  | identifier {
    $$ = new Node("Type", " Identifier", yylineno);
  };

statements:
  %empty {
    $$ = new Node("Statements", "", yylineno);
  }
  | statements statement {
    $1->children.push_back($2);
    $$ = $1;
  };

condition: expression {
  $$ = new Node("Condition", "", yylineno);
  $$->children.push_back($1);
};


statement:
  LC statements RC {
    $$ = $2;
  }
  | IF LP condition RP statement {
    $$ = new Node("IfStatement", "", yylineno);
    $$->children.push_back($3);
    $$->children.push_back($5);
  }
  | IF LP condition RP statement ELSE statement {
    $$ = new Node("IfElseStatement", "", yylineno);
    $$->children.push_back($3);
    $$->children.push_back($5);
    $$->children.push_back($7);
  }
  | WHILE LP expression RP statement {
    $$ = new Node("WhileStatement", "", yylineno);
    $$->children.push_back($3);
    $$->children.push_back($5);
  }
  | PRINT LP expression RP SEMCOL {
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
  };

expression:
  expression ANDEXPR expression {
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
    $$ = new Node("Int", $1, yylineno);
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
  };

argument_list:
  %empty {
    $$ = new Node("ArgumentList", "", yylineno);
  }
  | expression {
    $$ = $1;
  }
  | argument_list COMMA expression {
    $$ = new Node("ArgumentList", "", yylineno);
    $$->children.push_back($1);  // Add previous arguments
    $$->children.push_back($3);  // Add current argument
  };

identifier:
  STRLIT {
    $$ = new Node("Name", $1, yylineno);
  };