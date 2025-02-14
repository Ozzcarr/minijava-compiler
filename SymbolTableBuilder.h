#ifndef SYMBOL_TABLE_BUILDER_H
#define SYMBOL_TABLE_BUILDER_H

#include "Node.h"
#include "SymbolTable.h"

void processVarDeclarationList(Node *child, Class &cls);
void processMethodDeclarationList(Node *child, Class &cls);
void processClassDeclaration(Node *current, SymbolTable &symbolTable);
void buildSymbolTable(Node *root, SymbolTable &symbolTable);

#endif