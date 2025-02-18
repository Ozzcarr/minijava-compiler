#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "Node.h"
#include "SymbolTable.h"

class SemanticAnalyzer {
   public:
    SemanticAnalyzer(SymbolTable &symbolTable) : symbolTable(symbolTable), semanticErrors(0) {}

    void analyze(Node *root);

    int getSemanticErrors() const { return semanticErrors; }

   private:
    SymbolTable &symbolTable;
    int semanticErrors;

    void checkClass(Node *node);
    void checkMethod(Node *node, const Class &cls);
    void checkStatement(Node *node, const Method &method);
    void checkExpression(Node *node, const Method &method);
    std::string inferType(Node *expression);
    void reportError(const std::string &message, int lineno);
};

#endif  // SEMANTIC_ANALYZER_H