#include "SemanticAnalyzer.h"

#include <algorithm>

//Define colors for error messages, DEBUG ONLY
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define PURPLE  "\033[35m"

// This function starts the semantic analysis by traversing the AST from the root node.
void SemanticAnalyzer::analyze(Node *root) {
    // If the root node is null, return immediately.
    if (!root) return;

    // Check the root node for semantic correctness.
    Node *classDeclList = nullptr;
    for (auto child : root->children) {
        if (child->type == "ClassDeclarationList") {
            classDeclList = child;
            break;
        }
    }

    // Iterate over all children of the root node.
    if (classDeclList) {
        for (auto child : classDeclList->children) {
            if (child->type == "ClassDeclaration") {
                checkClass(child);
            }
        }
    } else {
        reportError("No class declaration list found.", root->lineno, RESET);
    }
}

// This function checks a class node for semantic correctness.
void SemanticAnalyzer::checkClass(Node *node) {
    // Get the class name from the node's value.
    std::string className = node->value;

    // Check if the class is declared in the symbol table.
    if (symbolTable.getClasses().find(className) == symbolTable.getClasses().end()) {
        // If the class is not declared, report an error and return.
        reportError("Class " + className + " is not declared.", node->lineno, RESET);
        return;
    }

    // Get the class object from the symbol table.
    const Class &cls = symbolTable.getClasses().at(className);

    // Check the root node for semantic correctness.
    Node *methodDeclList = nullptr;
    for (auto child : node->children) {
        if (child->type == "MethodDeclarationList") {
            methodDeclList = child;
            break;
        }
    }

    // Iterate over all children of the class node.
    if (methodDeclList) {
        for (auto child : methodDeclList->children) {
            if (child->type == "MethodDeclaration") {
                checkMethod(child, cls);
            }
        }
    } else {
        reportError("No method declaration list found in class " + className, node->lineno, RESET);
    }
}

// This function checks a method node for semantic correctness within a given class.
void SemanticAnalyzer::checkMethod(Node *node, const Class &cls) {
    // Get the method name from the node's value.
    std::string methodName = node->value;

    // Get the list of methods in the class.
    const auto &methods = cls.getMethods();

    // Find the method in the list of methods using a lambda function.
    auto it = std::find_if(methods.begin(), methods.end(), [&](const Method &m) { return m.getName() == methodName; });

    // If the method is not found, report an error and return.
    if (it == methods.end()) {
        reportError("Method " + methodName + " is not declared in class " + cls.getName(), node->lineno, RESET);
        return;
    }

    // Get the method object from the iterator.
    const Method &method = *it;

    // Check the root node for semantic correctness.
    Node *code = nullptr;
    for (auto child : node->children) {
        if (child->type == "Code") {
            code = child;
            break;
        }
    }

    if (!code) {
        return;
    }

    // Iterate over all children of the method node.
    for (auto child : code->children) {
        // If the child node's type ends with "Statement", check the statement.
        if (child->type.size() >= 9 && child->type.compare(child->type.size() - 9, 9, "Statement") == 0) {
            checkStatement(child, method, cls);
        }
        // If the child node's type ends with "Expression", check the expression.
        else if (child->type.size() >= 10 && child->type.compare(child->type.size() - 10, 10, "Expression") == 0) {
            checkExpression(child, method, cls);
        }
    }
}

// This function checks a statement node for semantic correctness within a given method.
void SemanticAnalyzer::checkStatement(Node *node, const Method &method, const Class &cls) {
    std::string statementType = node->type.substr(0, node->type.size() - 9);

    if (statementType == "VarInit") {
        // Variable initialization: Identifier "=" Expression ";"
        auto it = node->children.begin();

        Node *var = (*it);
        std::string varName = var->value;
        std::string varType = inferType(var, method, cls);
        Node *expression = *(++it);

        // Check the expression for semantic correctness
        checkExpression(expression, method, cls);
        std::string expressionType = inferType(expression, method, cls);
        if (varType != expressionType) {
            reportError("Assignment mismatch: variable '" + varName + "' is declared as " + varType + " but assigned "
                        + expressionType,
                        node->lineno, YELLOW);
        }

    } else {
        // Handle other statement types
        reportError("Unknown statement type: " + statementType, node->lineno, YELLOW);
    }
}

// Helper function to get the types of the left and right expressions in a binary operation.
std::pair<std::string, std::string> SemanticAnalyzer::getTypes(Node *node, const Method &method, const Class &cls) {
    auto it = node->children.begin();
    Node *left = *it;
    Node *right = *(++it);
    checkExpression(left, method, cls);
    checkExpression(right, method, cls);
    std::string leftType = inferType(left, method, cls);
    std::string rightType = inferType(right, method, cls);
    return {leftType, rightType};
}

// This function checks an expression node for semantic correctness within a given method.
void SemanticAnalyzer::checkExpression(Node *node, const Method &method, const Class &cls) {
    std::string expressionType = node->type;

    if (expressionType == "IntLiteral") {
        // Handle integer literal
    } else if (expressionType == "BoolLiteral") {
        // Handle boolean literal
    } else if (expressionType == "AddExpression" || expressionType == "SubExpression" ||
               expressionType == "MultExpression") {

        auto [leftType, rightType] = getTypes(node, method, cls);

        if (leftType != "Int" || rightType != "Int") {
            std::string op = " + ";
            if (expressionType == "SubExpression")
                op = " - ";
            else if (expressionType == "MultExpression")
                op = " * ";
            reportError( 
                "Type mismatch: arithmetic operations require integer operands. (" + leftType + op + rightType + ")",
                node->lineno, BLUE);
        }
    } else if (expressionType == "AndExpression" || expressionType == "OrExpression") {
        // Handle logical expressions
        auto [leftType, rightType] = getTypes(node, method, cls);

        if (leftType != "Bool" || rightType != "Bool") {
            std::string op = " && ";
            if (expressionType == "OrExpression") op = " || ";
            reportError(
                "Type mismatch: logical operations require boolean operands. (" + leftType + op + rightType + ")",
                node->lineno, RED);
        }
    } else if (expressionType == "EqualExpression") {
        // Handle comparison expressions
        auto [leftType, rightType] = getTypes(node, method, cls);
        std::string op = " == ";

        if (leftType != rightType) {
            reportError("Type mismatch: equality operations require operands of the same type. (" + leftType + " "
                        + op + " " + rightType + ")",
                        node->lineno, RED);
        } else if (leftType != "Int" && leftType != "Bool" && leftType != "IntArray") {
            reportError("Type mismatch: equality operations require int, bool or int array operands. (" + leftType
                        + op + rightType + ")",
                        node->lineno, RED);
        }
    } else if (expressionType == "LTExpression" || expressionType == "GTEXPRESSION") {
        auto [leftType, rightType] = getTypes(node, method, cls);

        if (leftType != "Int" || rightType != "Int") {
            std::string op = " < ";
            if (expressionType == "GTEXPRESSION") op = " > ";
            reportError(
                "Type mismatch: comparison operations require integer operands. (" + leftType + op + rightType + ")",
                node->lineno, RED);
        }
    } else if (expressionType == "NotExpression") {
        Node *left = *(node->children.begin());
        checkExpression(left, method, cls);
        std::string leftType = inferType(left, method, cls);
        if (leftType != "Bool") {
            reportError("Type mismatch: logical negation requires a boolean operand. (" + leftType + ")",
                        node->lineno, RED);
        }
    } else if (expressionType == "ArrayExpression") {
        auto [leftType, rightType] = getTypes(node, method, cls);
        if (leftType != "IntArray" || rightType != "Int") {
            reportError("Type mismatch: array access requires an integer index. (" + leftType + "[" + rightType + "]" + ")",
                        node->lineno, PURPLE);
        }
    }
}



// Helper function to infer the type of an expression
std::string SemanticAnalyzer::inferType(Node *expression, const Method &method, const Class &cls) {
    if (expression->type == "IntLiteral") {
        return "Int";
    } else if (expression->type == "BoolLiteral") {
        return "Bool";
    } else if (expression->type == "IntArray") {
        return "IntArray";
    } else if (expression->type == "Identifier") {
        std::string varName = expression->value;

        // Check if the identifier is a parameter of the method
        auto paramIt = std::find_if(method.getParameters().begin(), method.getParameters().end(),
                                    [&](const Variable &var) { return var.getName() == varName; });
        if (paramIt != method.getParameters().end()) {
            return paramIt->getType();
        }

        // Check if the identifier is a variable of the class
        auto varIt = std::find_if(cls.getVariables().begin(), cls.getVariables().end(),
                                  [&](const Variable &var) { return var.getName() == varName; });
        if (varIt != cls.getVariables().end()) {
            return varIt->getType();
        }

        // If the identifier is not found, report an error
        reportError("Variable '" + varName + "' is not declared in the method or class scope.", expression->lineno, RESET);
        return "";
    } else if (expression->type == "AddExpression" || expression->type == "SubExpression" ||
               expression->type == "MultExpression") {
        return "Int";
    } else if (expression->type == "AndExpression" || expression->type == "OrExpression") {
        return "Bool";
    } else if (expression->type == "EqualExpression" || expression->type == "LTExpression" ||
               expression->type == "GTExpression") {
        return "Bool";
    } else if (expression->type == "MethodCallExpression") {
        std::string methodName = expression->value;
        std::string classIdentifier = (*(expression->children.begin()))->value;
        std::string className;
        if (classIdentifier == "this") {
            className = cls.getName();
        } else {
            std::string className = symbolTable.getVariableType(classIdentifier);
        }
        return symbolTable.getMethodReturnType(className, methodName);
    } else if (expression->type == "NotExpression") {
        return "Bool";
    } else if (expression->type == "ArrayExpression") {
        return "Int";
    } else {
        return "";
    }
}

// This function reports a semantic error with a given message and line number.
void SemanticAnalyzer::reportError(const std::string &message, int lineno, const std::string &color) {
    // Print the error message to the standard error stream.
    std::cerr << color << "\@error at line " << lineno << ": " << message << RESET << std::endl;

    // Increment the count of semantic errors.
    semanticErrors++;
}