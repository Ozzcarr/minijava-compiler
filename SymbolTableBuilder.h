#ifndef SYMBOL_TABLE_BUILDER_H
#define SYMBOL_TABLE_BUILDER_H

#include "Node.h"
#include "SymbolTable.h"

/**
 * @brief Processes a variable declaration list and adds the variables to the class.
 * @param child The node representing the variable declaration list.
 * @param cls The class to which the variables will be added.
 */
void processVarDeclarationList(Node *child, Class &cls);

/**
 * @brief Processes a method declaration list and adds the methods to the class.
 * @param child The node representing the method declaration list.
 * @param cls The class to which the methods will be added.
 */
void processMethodDeclarationList(Node *child, Class &cls);

/**
 * @brief Processes a class declaration and adds the class to the symbol table.
 * @param current The node representing the class declaration.
 * @param symbolTable The symbol table to which the class will be added.
 */
void processClassDeclaration(Node *current, SymbolTable &symbolTable);

/**
 * @brief Builds the symbol table by processing the AST from the root node.
 * @param root The root node of the AST.
 * @param symbolTable The symbol table to be built.
 */
void buildSymbolTable(Node *root, SymbolTable &symbolTable);

#endif  // SYMBOL_TABLE_BUILDER_H