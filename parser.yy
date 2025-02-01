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
%token <std::string> PUBLIC CLASS STATIC VOID MAIN STRING RETURN INTARR BOOL INT IF ELSE WHILE PRINT EQUALSSIGN LP RP RC LC SEMCOL COMMA ANDEXPR OREXPR LTEXPR GTEXPR EQUALSEXPR PLUSOP MINUSOP MULTOP LB RB DOT LEN THIS NEW EXCLMARK TRUE FALSE INTLIT STRLIT
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
%type <Node *> root variable variable_list return code method_declaration type statement_list condition statement expression argument_list identifier next_row class_declaration var_declaration var_declaration_list method_declaration_list main_class class_declaration_list goal non_empty_variable_list non_empty_argument_list

/* Grammar rules section */
/* This section defines the production rules for the language being parsed */
%%
root:
    goal { root = $1; };

goal:
    main_class class_declaration_list END {
        $$ = new Node("Goal", "", yylineno);
        $$->children.push_back($1);
        $$->children.push_back($2);
    };

class_declaration_list:
    %empty {
        $$ = new Node("ClassDeclarationList", "", yylineno);
    }
    | class_declaration_list class_declaration {
        $$ = $1;
        $$->children.push_back($2);
    };

main_class:
    PUBLIC CLASS identifier LC PUBLIC STATIC VOID MAIN LP STRING LB RB identifier RP LC statement statement_list RC RC {
        $$ = new Node("MainClass", $3->value, yylineno);
        Node* stringArgs = new Node("StringArgs", $13->value, yylineno);
        $$->children.push_back(stringArgs);

        Node* statementsNode = new Node("Statements", "", yylineno);
        statementsNode->children.push_back($16);
        for (auto child : $17->children) {
            statementsNode->children.push_back(child);
        }
        $$->children.push_back(statementsNode);
    }

class_declaration:
    CLASS identifier LC var_declaration_list method_declaration_list RC {
        $$ = new Node("ClassDeclaration", $2->value, yylineno);
        $$->children.push_back($4);
        $$->children.push_back($5);
    };

var_declaration_list:
  %empty {
        $$ = new Node("VarDeclarationList", "", yylineno);
    }
    | var_declaration_list var_declaration {
        $$ = $1;
        $$->children.push_back($2);
    };

method_declaration_list:
    %empty {
        $$ = new Node("MethodDeclarationList", "", yylineno);
    }
    | method_declaration_list method_declaration {
        $$ = $1;
        $$->children.push_back($2);
    };

var_declaration:
    type identifier SEMCOL {
        $$ = new Node("VarDeclaration", "", yylineno);
        $$->children.push_back($1);
        $$->children.push_back($2);
    };

variable:
    type identifier {
        $$ = new Node("Variable", $2->value, yylineno);
        $$->children.push_back($1);
    };

variable_list:
    %empty {
        $$ = new Node("VariableList", "", yylineno);
    }
    | non_empty_variable_list {
        $$ = $1;
    };

non_empty_variable_list:
    variable {
        $$ = new Node("VariableList", "", yylineno);
        $$->children.push_back($1);
    }
    | non_empty_variable_list COMMA variable {
        $$ = $1;
        $$->children.push_back($3);
    };

return:
    RETURN expression SEMCOL {
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
    variable SEMCOL {
        $$ = $1;
    }
    | statement {
        $$ = $1;
    };

method_declaration:
    PUBLIC type identifier LP variable_list RP LC code return RC {
        $$ = new Node("MethodDeclaration", $3->value, yylineno);
        $$->children.push_back($2);
        $$->children.push_back($5);
        $$->children.push_back($8);
        $$->children.push_back($9);
    };

type:
    INTARR {
        $$ = new Node("Type", "IntArray", yylineno);
    }
    | BOOL {
        $$ = new Node("Type", "Bool", yylineno);
    }
    | INT {
        $$ = new Node("Type", "Int", yylineno);
    }
    | identifier {
        $$ = new Node("Type", "Identifier", yylineno);
    };

statement_list:
    %empty {
        $$ = new Node("StatementList", "", yylineno);
    }
    | statement_list statement {
        $1->children.push_back($2);
        $$ = $1;
    };

condition:
    expression {
        $$ = new Node("Condition", "", yylineno);
        $$->children.push_back($1);
    };

statement:
    LC statement_list RC {
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
        $$ = new Node("PrintStatement", "", yylineno);
        $$->children.push_back($3);
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
        $$ = new Node("LengthExpression", "", yylineno);
        $$->children.push_back($1);
    }
    | expression DOT identifier LP argument_list RP {
        $$ = new Node("MethodCallExpression", $3->value, yylineno);
        $$->children.push_back($1);
        $$->children.push_back($5);
    }
    | INTLIT {
        $$ = new Node("IntLiteral", $1, yylineno);
    }
    | TRUE {
        $$ = new Node("BoolLiteral", "true", yylineno);
    }
    | FALSE {
        $$ = new Node("BoolLiteral", "false", yylineno);
    }
    | identifier {
        $$ = $1;
    }
    | THIS {
        $$ = new Node("ThisExpression", "this", yylineno);
    }
    | NEW INT LB expression RB {
        $$ = new Node("NewIntArrayExpression", "", yylineno);
        $$->children.push_back($4);
    }
    | NEW identifier LP RP {
        $$ = new Node("NewObjectExpression", "", yylineno);
        $$->children.push_back($2);
    }
    | EXCLMARK expression {
        $$ = new Node("NotExpression", "", yylineno);
        $$->children.push_back($2);
    }
    | LP expression RP {
        $$ = $2;
    };

argument_list:
    %empty {
        $$ = new Node("ArgumentList", "", yylineno);
    }
    | non_empty_argument_list {
        $$ = $1;
    };

non_empty_argument_list:
    expression {
        $$ = new Node("ArgumentList", "", yylineno);
        $$->children.push_back($1);
    }
    | non_empty_argument_list COMMA expression {
        $$ = $1;
        $$->children.push_back($3);
    };

identifier:
    STRLIT {
        $$ = new Node("Identifier", $1, yylineno);
    };