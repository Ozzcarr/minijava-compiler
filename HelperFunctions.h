#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H
#include "Node.h"

// Define colors for error messages
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define PURPLE "\033[35m"

/**
 * @brief Finds a child node with a specific type and occurrence.
 * @param node The parent node.
 * @param type The type of the child node to find.
 * @param occurrence The occurrence of the child node to find.
 * @return The child node if found, otherwise nullptr.
 */
inline Node *findChild(Node *node, const std::string &type, int occurrence = 1) {
    int count = 1;
    for (auto child : node->children) {
        if (child->type == type) {
            if (count == occurrence) {
                return child;
            }
            count++;
        }
    }
    return nullptr;
}

/**
 * @brief Checks if a string ends with a specific suffix.
 * @param str The string to check.
 * @param suffix The suffix to check for.
 * @return True if the string ends with the suffix, otherwise false.
 */
inline bool endsWith(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * @brief Checks if an expression type is a unary expression.
 * @param type The expression type to check.
 * @return True if the expression type is a unary expression, otherwise false.
 */
inline bool isUnaryExpression(const std::string &type) {
    return type == "NotExpression" || type == "LengthExpression" || type == "NewIntArrayExpression";
}

/**
 * @brief Checks if an expression type is an arithmetic expression.
 * @param type The expression type to check.
 * @return True if the expression type is an arithmetic expression, otherwise false.
 */
inline bool isArithmeticExpression(const std::string &type) {
    return type == "AddExpression" || type == "SubExpression" || type == "MultExpression";
}

/**
 * @brief Checks if an expression type is a logical expression.
 * @param type The expression type to check.
 * @return True if the expression type is a logical expression, otherwise false.
 */
inline bool isLogicalExpression(const std::string &type) { return type == "AndExpression" || type == "OrExpression"; }

/**
 * @brief Checks if an expression type is a comparison expression.
 * @param type The expression type to check.
 * @return True if the expression type is a comparison expression, otherwise false.
 */
inline bool isComparisonExpression(const std::string &type) { return type == "LTExpression" || type == "GTExpression"; }

/**
 * @brief Checks if an expression type is a binary expression.
 * @param type The expression type to check.
 * @return True if the expression type is a binary expression, otherwise false.
 */
inline bool isBinaryExpression(const std::string &type) {
    return isArithmeticExpression(type) || isLogicalExpression(type) || isComparisonExpression(type) ||
           type == "EqualExpression" || type == "ArrayExpression";
}

/**
 * @brief Checks if a type is valid for equality operations.
 * @param type The type to check.
 * @return True if the type is valid for equality operations, otherwise false.
 */
inline bool isValidEqualityType(const std::string &type) {
    return type == "Int" || type == "Bool" || type == "IntArray";
}

inline bool isLiteral(const std::string &type) { return type == "IntLiteral" || type == "BoolLiteral"; }
inline bool isIdentifier(const std::string &type) { return type == "Identifier"; }
inline bool isThisExpression(const std::string &type) { return type == "ThisExpression"; }

/**
 * @brief Gets the operator string for a given expression type.
 * @param expressionType The expression type.
 * @return The operator string.
 */
inline std::string getOperator(const std::string &expressionType) {
    if (expressionType == "AddExpression") return " + ";
    if (expressionType == "SubExpression") return " - ";
    if (expressionType == "MultExpression") return " * ";
    if (expressionType == "AndExpression") return " && ";
    if (expressionType == "OrExpression") return " || ";
    if (expressionType == "LTExpression") return " < ";
    if (expressionType == "GTExpression") return " > ";
    if (expressionType == "EqualExpression") return " == ";
    if (expressionType == "ArrayExpression") return "[";
    if (expressionType == "NotExpression") return "!";
    throw std::runtime_error("Couldn't get operator for expression type: " + expressionType);
}

/**
 * @brief Gets the color string for a given expression type.
 * @param expressionType The expression type.
 * @return The color string.
 */
inline std::string getColor(const std::string &expressionType) {
    if (isArithmeticExpression(expressionType)) return BLUE;
    if (isLogicalExpression(expressionType)) return RED;
    if (isComparisonExpression(expressionType)) return RED;
    return RED;
}

#endif  // HELPERFUNCTIONS_H