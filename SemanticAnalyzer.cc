#include "SemanticAnalyzer.h"

#include <algorithm>

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
        reportError("No class declaration list found.", root->lineno);
    }
}

// This function checks a class node for semantic correctness.
void SemanticAnalyzer::checkClass(Node *node) {
    // Get the class name from the node's value.
    std::string className = node->value;

    // Check if the class is declared in the symbol table.
    if (symbolTable.getClasses().find(className) == symbolTable.getClasses().end()) {
        // If the class is not declared, report an error and return.
        reportError("Class " + className + " is not declared.", node->lineno);
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
        reportError("No method declaration list found in class " + className, node->lineno);
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
        reportError("Method " + methodName + " is not declared in class " + cls.getName(), node->lineno);
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
        std::string varName = (*it)->value;
        Node *expression = *(++it);

        // Check the expression for semantic correctness
        checkExpression(expression, method, cls);

    } else {
        // Handle other statement types
        reportError("Unknown statement type: " + statementType, node->lineno);
    }
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
        auto it = node->children.begin();
        Node *left = *it;
        Node *right = *(++it);
        checkExpression(left, method, cls);
        checkExpression(right, method, cls);
        std::string leftType = inferType(left, method, cls);
        std::string rightType = inferType(right, method, cls);

        if (leftType != "Int" || rightType != "Int") {
            std::string op = " + ";
            if (expressionType == "SubExpression")
                op = " - ";
            else if (expressionType == "MultExpression")
                op = " * ";
            reportError(
                "Type mismatch: arithmetic operations require integer operands. (" + leftType + op + rightType + ")",
                node->lineno);
        }
    } else if (expressionType == "AndExpression" || expressionType == "OrExpression") {
        // Handle logical expressions
        auto it = node->children.begin();
        Node *left = *it;
        Node *right = *(++it);
        checkExpression(left, method, cls);
        checkExpression(right, method, cls);
        std::string leftType = inferType(left, method, cls);
        std::string rightType = inferType(right, method, cls);
        if (leftType != "Bool" || rightType != "Bool") {
            std::string op = " && ";
            if (expressionType == "OrExpression") op = " || ";
            reportError(
                "Type mismatch: logical operations require boolean operands. (" + leftType + op + rightType + ")",
                node->lineno);
        }
    }
    // } else if (expressionType == "AndExpression" || expressionType == "OrExpression") {
    //     // Handle logical expressions
    //     auto it = node->children.begin();
    //     Node *left = *it;
    //     Node *right = *(++it);
    //     checkExpression(left, method, cls);
    //     checkExpression(right, method, cls);
    //     std::string leftType = inferType(left);
    //     std::string rightType = inferType(right);
    //     if (leftType != "boolean" || rightType != "boolean") {
    //         reportError("Type mismatch: logical operations require boolean operands.", node->lineno);
    //     }
    // } else if (expressionType == "EqualExpression" || expressionType == "LTExpression" ||
    //            expressionType == "GTExpression") {
    //     // Handle comparison expressions
    //     auto it = node->children.begin();
    //     Node *left = *it;
    //     Node *right = *(++it);
    //     checkExpression(left, method, cls);
    //     checkExpression(right, method, cls);
    //     std::string leftType = inferType(left);
    //     std::string rightType = inferType(right);
    //     if (leftType != rightType) {
    //         reportError("Type mismatch: comparison operations require operands of the same type.", node->lineno);
    //     }
    // } else if (expressionType == "MethodCallExpression") {
    //     // Handle method call expressions
    //     std::string methodName = node->value;
    //     auto it = node->children.begin();
    //     Node *object = *it;
    //     Node *arguments = *(++it);
    //     checkExpression(object, method, cls);
    //     for (auto arg : arguments->children) {
    //         checkExpression(arg, method, cls);
    //     }
    //     // Additional checks for method call can be added here
    // } else {
    //     // Handle other expression types
    //     reportError("Unknown expression type: " + expressionType, node->lineno);
    // }
    // }
}

// Helper function to infer the type of an expression
std::string SemanticAnalyzer::inferType(Node *expression, const Method &method, const Class &cls) {
    if (expression->type == "IntLiteral") {
        return "Int";
    } else if (expression->type == "BoolLiteral") {
        return "Bool";
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
        reportError("Variable '" + varName + "' is not declared in the method or class scope.", expression->lineno);
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
        // Assuming the return type of the method call is already known
        return "Int";
    } else {
        return "";
    }
}

// This function reports a semantic error with a given message and line number.
void SemanticAnalyzer::reportError(const std::string &message, int lineno) {
    // Print the error message to the standard error stream.
    std::cerr << "Semantic error at line " << lineno << ": " << message << std::endl;

    // Increment the count of semantic errors.
    semanticErrors++;
}