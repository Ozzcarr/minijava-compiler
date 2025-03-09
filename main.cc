#include <iostream>
#include <stack>
#include <fstream>

#include "IntermediateRepresentation.h"
#include "BytecodeGenerator.h"
#include "Node.h"
#include "SemanticAnalyzer.h"
#include "SymbolTable.h"
#include "SymbolTableBuilder.h"
#include "parser.tab.hh"

extern Node *root;
extern FILE *yyin;
extern int yylineno;
extern int lexical_errors;
extern yy::parser::symbol_type yylex();

enum errCodes {
    SUCCESS = 0,
    LEXICAL_ERROR = 1,
    SYNTAX_ERROR = 2,
    AST_ERROR = 3,
    SEMANTIC_ERROR = 4,
    IR_ERROR = 5,
    SEGMENTATION_FAULT = 139
};

int errCode = errCodes::SUCCESS;

// Handling Syntax Errors.
void yy::parser::error(std::string const &err) {
    if (!lexical_errors) {
        std::cerr << "Syntax errors found! See the logs below:" << std::endl;
        std::cerr << "\t@error at line " << yylineno << ". Cannot generate a syntax for this input:" << err.c_str()
                  << std::endl;
        std::cerr << "End of syntax errors!" << std::endl;
        errCode = errCodes::SYNTAX_ERROR;
    }
}

// Prints the symbol table.
void printSymbolTable(SymbolTable &symbolTable) {
    for (const auto &cls : symbolTable.getClasses()) {
        std::cout << "Class: " << cls.second.getName() << std::endl;
        for (const auto &var : cls.second.getVariables()) {
            std::cout << "  Variable: " << var.getName() << " of type " << var.getType() << std::endl;
        }
        for (const auto &method : cls.second.getMethods()) {
            std::cout << "  Method: " << method.getName() << " returns " << method.getReturnType() << std::endl;
            for (const auto &param : method.getParameters()) {
                std::cout << "    Param: " << param.getName() << " of type " << param.getType() << std::endl;
            }
            for (const auto &localVarPair : method.getLocalVariables()) {
                std::cout << "    Local Variable: " << localVarPair.first.getName() << " of type "
                          << localVarPair.first.getType() << " on line " << localVarPair.second << std::endl;
            }
        }
    }
}

// Exit with error code.
void exitWithError(int errorCode) {
    std::cout << "\nExiting with code: " << errorCode << std::endl;
    exit(errorCode);
}

int main(int argc, char **argv) {
    // Reads from file if a file name is passed as an argument. Otherwise, reads from stdin.
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }

    if (USE_LEX_ONLY) {
        yylex();
    } else {
        yy::parser parser;

        bool parseSuccess = !parser.parse();

        if (lexical_errors) {
            exitWithError(errCodes::LEXICAL_ERROR);
        }

        if (!parseSuccess) {
            exitWithError(errCodes::SYNTAX_ERROR);
        }

        std::cout << "\nThe compiler successfully generated a syntax tree for the given input!\n";

        // Generate the AST
        try {
            root->generate_tree();
        } catch (const std::exception &e) {
            std::cerr << "Error generating tree: " << e.what() << std::endl;
            exitWithError(errCodes::AST_ERROR);
        }

        // Create symbol table
        SymbolTable symbolTable;
        try {
            buildSymbolTable(root, symbolTable);
            printSymbolTable(symbolTable);
        } catch (const std::exception &e) {
            std::cerr << "Error building symbol table: " << e.what() << std::endl;
            exitWithError(errCodes::AST_ERROR);
        }

        std::cout << "\n";

        // Perform semantic analysis
        try {
            SemanticAnalyzer semanticAnalyzer(symbolTable);
            semanticAnalyzer.analyze(root);

            if (semanticAnalyzer.getSemanticErrors() > 0) {
                std::cout << "\nSemantic errors found: " << semanticAnalyzer.getSemanticErrors() << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << "Error during semantic analysis: " << e.what() << std::endl;
            exitWithError(errCodes::SEMANTIC_ERROR);
        }

        // Generate intermediate representation
        ControlFlowGraph cfg;
        try {
            cfg.traverseAST(root);
            cfg.writeCFG();
        } catch (const std::exception &e) {
            std::cerr << "Error generating intermediate representation: " << e.what() << std::endl;
            exitWithError(errCodes::IR_ERROR);
        }

        // Generate bytecode
        try {
            BCProgram program;
            program.generateBytecode(cfg, symbolTable);

            std::ofstream outFile("output.bc");
            if (!outFile) {
                throw std::runtime_error("Failed to open output.bc for writing");
            }
            program.print(outFile);
        } catch (const std::exception &e) {
            std::cerr << "Error generating bytecode: " << e.what() << std::endl;
            exitWithError(errCodes::IR_ERROR);
        }
    }

    exitWithError(0);
}