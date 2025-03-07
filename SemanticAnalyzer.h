#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "HelperFunctions.h"
#include "Node.h"
#include "SymbolTable.h"

class SemanticAnalyzer {
   public:
    SemanticAnalyzer(SymbolTable &symbolTable) : symbolTable(symbolTable), semanticErrors(0) {}

    /**
     * @brief Starts the semantic analysis by traversing the AST from the root node.
     * @param root The root node of the AST.
     */
    void analyze(Node *root);

    /**
     * @brief Returns the number of semantic errors found.
     * @return The number of semantic errors.
     */
    int getSemanticErrors() const { return semanticErrors; }

   private:
    SymbolTable &symbolTable;
    int semanticErrors;

    // Main analysis functions

    /**
     * @brief Checks a class node for semantic correctness.
     * @param node The class node to check.
     * @param clsNames The names of the classes to check against to verify no duplicate identifiers.
     */
    void checkClass(Node *node, std::vector<std::string> &clsNames);

    /**
     * @brief Checks a method node for semantic correctness within a given class.
     * @param node The method node to check.
     * @param cls The class containing the method.
     * @param clsNames The names of the classes to check against to verify no duplicate identifiers.
     * @param classVars The names of the class variables to check against.
     * @param methodNames The names of the methods to check against to verify no duplicate identifiers.
     */
    void checkMethod(Node *node, const Class &cls);

    /**
     * @brief Checks a statement node for semantic correctness within a given method.
     * @param node The statement node to check.
     * @param method The method containing the statement.
     * @param cls The class containing the method.
     */
    void checkStatement(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Checks a variable initialization statement for semantic correctness within a given method.
     * @param node The variable initialization statement node to check.
     * @param method The method containing the statement.
     * @param cls The class containing the method.
     */
    void checkVarInitStatement(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Checks an array initialization statement for semantic correctness within a given method.
     * @param node The array initialization statement node to check.
     * @param method The method containing the statement.
     * @param cls The class containing the method.
     */
    void checkArrayInitStatement(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Checks an if statement for semantic correctness within a given method.
     * @param node The if statement node to check.
     * @param method The method containing the statement.
     * @param cls The class containing the method.
     */
    void checkIfStatement(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Checks an if-else statement for semantic correctness within a given method.
     * @param node The if-else statement node to check.
     * @param method The method containing the statement.
     * @param cls The class containing the method.
     */
    void checkIfElseStatement(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Checks a while statement for semantic correctness within a given method.
     * @param node The while statement node to check.
     * @param method The method containing the statement.
     * @param cls The class containing the method.
     */
    void checkWhileStatement(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Checks a print statement for semantic correctness within a given method.
     * @param node The print statement node to check.
     * @param method The method containing the statement.
     * @param cls The class containing the method.
     */
    void checkPrintStatement(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Checks an expression node for semantic correctness within a given method.
     * @param node The expression node to check.
     * @param method The method containing the expression.
     * @param cls The class containing the method.
     */
    void checkExpression(Node *node, const Method &method, const Class &cls);

    // Helper functions

    /**
     * @brief Checks a binary expression node for semantic correctness.
     * @param node The binary expression node to check.
     * @param method The method containing the expression.
     * @param cls The class containing the method.
     * @param expressionType The type of the binary expression.
     */
    void checkBinaryExpression(Node *node, const Method &method, const Class &cls, const std::string &expressionType);

    /**
     * @brief Checks a binary expression node for semantic correctness with expected types.
     * @param node The binary expression node to check.
     * @param method The method containing the expression.
     * @param cls The class containing the method.
     * @param expressionType The type of the binary expression.
     * @param expectedLeftType The expected type of the left operand.
     * @param expectedRightType The expected type of the right operand.
     * @param errorMessage The error message to report if the types do not match.
     */
    void checkBinaryExpression(Node *node, const Method &method, const Class &cls, const std::string &expressionType,
                               const std::string &expectedLeftType, const std::string &expectedRightType,
                               const std::string &errorMessage);

    /**
     * @brief Checks a unary expression node for semantic correctness.
     * @param node The unary expression node to check.
     * @param method The method containing the expression.
     * @param cls The class containing the method.
     * @param expressionType The type of the unary expression.
     */
    void checkUnaryExpression(Node *node, const Method &method, const Class &cls, const std::string &expressionType);

    /**
     * @brief Checks a unary expression node for semantic correctness with expected type.
     * @param node The unary expression node to check.
     * @param method The method containing the expression.
     * @param cls The class containing the method.
     * @param expectedType The expected type of the operand.
     * @param errorMessage The error message to report if the type does not match.
     */
    void checkUnaryExpression(Node *node, const Method &method, const Class &cls, const std::string &expectedType,
                              const std::string &errorMessage);

    /**
     * @brief Checks a method call node for semantic correctness.
     * @param node The method call node to check.
     * @param method The method containing the call.
     * @param cls The class containing the method.
     */
    void checkMethodCallArguments(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Gets the types of the left and right expressions in a binary operation.
     * @param node The binary expression node.
     * @param method The method containing the expression.
     * @param cls The class containing the method.
     * @return A pair of strings representing the types of the left and right expressions.
     */
    std::pair<std::string, std::string> getTypes(Node *node, const Method &method, const Class &cls);

    /**
     * @brief Infers the type of an expression.
     * @param expression The expression node.
     * @param method The method containing the expression.
     * @param cls The class containing the method.
     * @return The inferred type of the expression.
     */
    std::string inferType(Node *expression, const Method &method, const Class &cls);

    /**
     * @brief Infers the type of an identifier.
     * @param expression The identifier node.
     * @param method The method containing the identifier.
     * @param cls The class containing the method.
     * @return The inferred type of the identifier.
     */
    std::string inferIdentifierType(Node *expression, const Method &method, const Class &cls);

    /**
     * @brief Reports a semantic error with a given message and line number.
     * @param message The error message.
     * @param lineno The line number where the error occurred.
     * @param color The color to use for the error message.
     */
    void reportError(const std::string &message, int lineno, const std::string &color);
};

#endif  // SEMANTIC_ANALYZER_H