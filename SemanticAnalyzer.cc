#include "SemanticAnalyzer.h"

// Main analysis functions

void SemanticAnalyzer::analyze(Node *root) {
    if (!root) throw std::runtime_error("Root node is null.");

    Node *classDeclList = findChild(root, "ClassDeclarationList");
    if (classDeclList) {
        for (auto child : classDeclList->children) {
            if (child->type == "ClassDeclaration") {
                checkClass(child);
            }
        }
    } else {
        throw std::runtime_error("No class declaration list found in the AST.");
    }
}

void SemanticAnalyzer::checkClass(Node *node) {
    std::string className = node->value;

    if (!symbolTable.hasClass(className)) {
        reportError("Class " + className + " is not declared.", node->lineno, RESET);
        return;
    }

    const Class &cls = symbolTable.getClass(className);
    Node *methodDeclList = findChild(node, "MethodDeclarationList");

    if (!methodDeclList) throw std::runtime_error("No method declaration list found in class " + className);

    for (auto child : methodDeclList->children) {
        if (child->type == "MethodDeclaration") {
            checkMethod(child, cls);
        }
    }

    // Check that class variables are of existing types
    Node *varDeclList = findChild(node, "VarDeclarationList");
    if (varDeclList) {
        for (auto varNode : varDeclList->children) {
            auto it = varNode->children.begin();
            std::string varType = (*it)->value;
            std::string varName = (*(++it))->value;
            int varLineno = varNode->lineno;

            if (varType != "Int" && varType != "Bool" && varType != "IntArray") {
                if (!symbolTable.hasClass(varType)) {
                    reportError("Class variable " + varName + " has an invalid type: " + varType, varLineno, RESET);
                }
            }
        }
    }
}

void SemanticAnalyzer::checkMethod(Node *node, const Class &cls) {
    std::string methodName = node->value;

    if (!cls.hasMethod(methodName)) {
        reportError("Method " + methodName + " is not declared in class " + cls.getName(), node->lineno, RESET);
        return;
    }

    const Method &method = cls.getMethod(methodName);

    // Check method code
    Node *code = findChild(node, "Code");
    if (!code) throw std::runtime_error("No code block found in method " + methodName);

    for (auto child : code->children) {
        if (endsWith(child->type, "Statement")) {
            checkStatement(child, method, cls);
        } else if (child->type != "Variable") {
            throw std::runtime_error("Method code must contain statements. Unexpected node type: " + child->type);
        }
    }

    // Check return statement
    Node *returnStatement = findChild(node, "Return");
    if (!returnStatement) throw std::runtime_error("No return statement found in method " + methodName);

    Node *returnExpression = returnStatement->children.front();
    if (!returnExpression) throw std::runtime_error("No expression found in return statement of method " + methodName);

    checkExpression(returnExpression, method, cls);
    std::string returnType = inferType(returnExpression, method, cls);
    if (returnType != method.getReturnType()) {
        reportError("Return type mismatch: expected " + method.getReturnType() + " but got " + returnType, node->lineno,
                    RED);
    }
}

void SemanticAnalyzer::checkStatement(Node *node, const Method &method, const Class &cls) {
    std::string statementType = node->type.substr(0, node->type.size() - 9);

    if (statementType == "VarInit") {
        auto it = node->children.begin();
        Node *var = (*it);
        std::string varName = var->value;
        std::string varType = inferType(var, method, cls);

        Node *expression = *(++it);
        checkExpression(expression, method, cls);
        std::string expressionType = inferType(expression, method, cls);
        if (varType == "" || expressionType == "") return;

        if (varType != expressionType) {
            reportError("Assignment mismatch: variable '" + varName + "' is declared as " + varType + " but assigned " +
                            expressionType,
                        node->lineno, YELLOW);
        }

        // Check if the variable is a local variable and declared before it is used
        if (method.isLocalVariable(varName)) {
            if (!method.isVariableDeclaredBefore(varName, node->lineno)) {
                reportError("Variable '" + varName + "' is used before it is declared.", node->lineno, RED);
            }
        }
    } else if (statementType == "ArrayInit") {
        // Get children
        if (node->children.size() != 3) throw std::runtime_error("Array initialization must have exactly 3 children");
        auto it = node->children.begin();
        Node *var = *it;
        Node *size = *(++it);
        Node *expression = *(++it);

        // Check variable
        std::string varName = var->value;
        std::string varType = inferType(var, method, cls);
        if (varType != "IntArray") {
            reportError("Array initialization mismatch: variable '" + varName + "' is declared as " + varType +
                            " but assigned IntArray",
                        node->lineno, YELLOW);
        }

        // Check size
        checkExpression(size, method, cls);
        std::string sizeType = inferType(size, method, cls);
        if (sizeType != "Int") {
            reportError("Array size must be of type Int, but got " + sizeType, node->lineno, RED);
        }

        // Check expression
        checkExpression(expression, method, cls);
        std::string expressionType = inferType(expression, method, cls);
        if (expressionType != "Int") {
            reportError("Array initialization mismatch: variable '" + varName + "' is declared as " + varType +
                            " but assigned " + expressionType,
                        node->lineno, YELLOW);
        }
    } else if (statementType == "If") {
        // Check condition
        Node *condition = findChild(node, "Condition");
        if (!condition) throw std::runtime_error("No condition found in if statement");

        if (condition->children.size() != 1) throw std::runtime_error("If condition must have exactly one expression");

        Node *conditionExpression = condition->children.front();
        if (!conditionExpression) throw std::runtime_error("No expression found in condition");

        checkExpression(conditionExpression, method, cls);
        std::string conditionType = inferType(conditionExpression, method, cls);

        if (conditionType != "Bool") {
            reportError("Condition must be of type Bool, but got " + conditionType, node->lineno, RED);
        }

        // Check statements
        Node *statementList = findChild(node, "StatementList");
        if (!statementList) throw std::runtime_error("No statement list found in if statement");

        for (auto child : statementList->children) {
            checkStatement(child, method, cls);
        }
    } else if (statementType == "IfElse") {
        // Check condition
        Node *condition = findChild(node, "Condition");
        if (!condition) throw std::runtime_error("No condition found in if-else statement");

        if (condition->children.size() != 1) throw std::runtime_error("If condition must have exactly one expression");

        Node *conditionExpression = condition->children.front();
        if (!conditionExpression) throw std::runtime_error("No expression found in condition");

        checkExpression(conditionExpression, method, cls);
        std::string conditionType = inferType(conditionExpression, method, cls);

        if (conditionType != "Bool") {
            reportError("Condition must be of type Bool, but got " + conditionType, node->lineno, RED);
        }

        // Check statements
        Node *statementList = findChild(node, "StatementList");
        if (!statementList) throw std::runtime_error("No statement list found in if-else statement");

        for (auto child : statementList->children) {
            checkStatement(child, method, cls);
        }

        Node *elseStatementList = findChild(node, "StatementList", 2);
        if (!elseStatementList) throw std::runtime_error("No else statement list found in if-else statement");
        for (auto child : elseStatementList->children) {
            checkStatement(child, method, cls);
        }
    } else if (statementType == "While") {
        // TODO Implement
    } else if (statementType == "Print") {
        // TODO Implement
    } else {
        throw std::runtime_error("Unknown statement type: " + statementType + " on line " +
                                 std::to_string(node->lineno));
    }
}

void SemanticAnalyzer::checkExpression(Node *node, const Method &method, const Class &cls) {
    std::string expressionType = node->type;

    if (isBinaryExpression(expressionType)) {
        checkBinaryExpression(node, method, cls, expressionType);
    } else if (isUnaryExpression(expressionType)) {
        checkUnaryExpression(node, method, cls, expressionType);
    } else if (expressionType == "MethodCallExpression") {
        checkMethodCallArguments(node, method, cls);
        for (auto child : node->children) {
            checkExpression(child, method, cls);
        }
    } else if (expressionType == "NewObjectExpression") {
        if (node->children.size() != 1) throw std::runtime_error("NewObjectExpression must have exactly one child");
        std::string className = node->children.front()->value;
        if (!symbolTable.hasClass(className)) {
            reportError("Class " + className + " is not declared.", node->lineno, RED);
        }
        // } else if (expressionType == "Return") {
        // TODO Implement
    } else if (expressionType != "ArgumentList" && expressionType != "IntLiteral" && expressionType != "BoolLiteral" &&
               expressionType != "Identifier" && expressionType != "ThisExpression") {
        throw std::runtime_error("Unknown expression type: " + expressionType);
    }
}

// Helper functions

Node *SemanticAnalyzer::findChild(Node *node, const std::string &type, int occurrence) {
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

Node *SemanticAnalyzer::findChild(Node *node, const std::string &type) {
    for (auto child : node->children) {
        if (child->type == type) {
            return child;
        }
    }
    return nullptr;
}

bool SemanticAnalyzer::endsWith(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool SemanticAnalyzer::isBinaryExpression(const std::string &type) {
    return isArithmeticExpression(type) || isLogicalExpression(type) || isComparisonExpression(type) ||
           type == "EqualExpression" || type == "ArrayExpression";
}

bool SemanticAnalyzer::isUnaryExpression(const std::string &type) {
    return type == "NotExpression" || type == "LengthExpression" || type == "NewIntArrayExpression";
}

bool SemanticAnalyzer::isArithmeticExpression(const std::string &type) {
    return type == "AddExpression" || type == "SubExpression" || type == "MultExpression";
}

bool SemanticAnalyzer::isLogicalExpression(const std::string &type) {
    return type == "AndExpression" || type == "OrExpression";
}

bool SemanticAnalyzer::isComparisonExpression(const std::string &type) {
    return type == "LTExpression" || type == "GTExpression";
}

bool SemanticAnalyzer::isValidEqualityType(const std::string &type) {
    return type == "Int" || type == "Bool" || type == "IntArray";
}

void SemanticAnalyzer::checkBinaryExpression(Node *node, const Method &method, const Class &cls,
                                             const std::string &expressionType) {
    if (isArithmeticExpression(expressionType)) {
        checkBinaryExpression(node, method, cls, expressionType, "Int", "Int",
                              "arithmetic operations require integer operands");
    } else if (isLogicalExpression(expressionType)) {
        checkBinaryExpression(node, method, cls, expressionType, "Bool", "Bool",
                              "logical operations require boolean operands");
    } else if (isComparisonExpression(expressionType)) {
        checkBinaryExpression(node, method, cls, expressionType, "Int", "Int",
                              "comparison operations require integer operands");
    } else if (expressionType == "ArrayExpression") {
        checkBinaryExpression(node, method, cls, expressionType, "IntArray", "Int",
                              "array access requires an integer index");
    } else if (expressionType == "EqualExpression") {
        auto [leftType, rightType] = getTypes(node, method, cls);
        if (leftType != rightType || !isValidEqualityType(leftType)) {
            reportError("Type mismatch: equality operations require operands of the same type. (" + leftType +
                            " == " + rightType + ")",
                        node->lineno, RED);
        }
    } else {
        throw std::runtime_error("Unknown binary expression type: " + expressionType + " on line " +
                                 std::to_string(node->lineno));
    }
}

void SemanticAnalyzer::checkBinaryExpression(Node *node, const Method &method, const Class &cls,
                                             const std::string &expressionType, const std::string &expectedLeftType,
                                             const std::string &expectedRightType, const std::string &errorMessage) {
    auto [leftType, rightType] = getTypes(node, method, cls);
    if (leftType != expectedLeftType || rightType != expectedRightType) {
        std::string suffix = expressionType == "ArrayExpression" ? "]" : "";
        reportError("Type mismatch: " + errorMessage + ". (" + leftType + getOperator(expressionType) + rightType +
                        suffix + ")",
                    node->lineno, getColor(expressionType));
    }
}

void SemanticAnalyzer::checkUnaryExpression(Node *node, const Method &method, const Class &cls,
                                            const std::string &expressionType) {
    if (expressionType == "NotExpression") {
        checkUnaryExpression(node, method, cls, "Bool", "logical negation requires a boolean operand");
    } else if (expressionType == "LengthExpression") {
        checkUnaryExpression(node, method, cls, "IntArray", "length operation requires an integer array operand");
    } else if (expressionType == "NewIntArrayExpression") {
        checkUnaryExpression(node, method, cls, "Int", "new int array requires an integer size");
    } else {
        throw std::runtime_error("Unknown unary expression type: " + expressionType + " on line " +
                                 std::to_string(node->lineno));
    }
}

void SemanticAnalyzer::checkUnaryExpression(Node *node, const Method &method, const Class &cls,
                                            const std::string &expectedType, const std::string &errorMessage) {
    Node *child = node->children.front();
    checkExpression(child, method, cls);
    std::string childType = inferType(child, method, cls);
    if (childType != expectedType) {
        reportError("Type mismatch: " + errorMessage + ". (" + childType + ")", node->lineno, RED);
    }
}

void SemanticAnalyzer::checkMethodCallArguments(Node *node, const Method &method, const Class &cls) {
    std::string methodName = node->value;

    // Find the class of the object on which the method is called
    Node *objectNode = node->children.front();
    std::string objectType = inferType(objectNode, method, cls);

    if (!symbolTable.hasClass(objectType)) {
        reportError("Class " + objectType + " is not declared.", node->lineno, RED);
        return;
    }

    const Class &objectClass = symbolTable.getClass(objectType);

    if (!objectClass.hasMethod(methodName)) {
        reportError("Method " + methodName + " not found in class " + objectType, node->lineno, RED);
        return;
    }

    const Method &calledMethod = objectClass.getMethod(methodName);

    Node *argumentsNode = findChild(node, "ArgumentList");
    const auto &parameters = calledMethod.getParameters();

    if (!argumentsNode) {
        if (!parameters.empty()) {
            reportError("Method " + methodName + " expects " + std::to_string(parameters.size()) +
                            " arguments, but none were provided.",
                        node->lineno, RED);
        }
        return;
    }

    const auto &arguments = argumentsNode->children;

    if (parameters.size() != arguments.size()) {
        reportError("Method " + methodName + " expects " + std::to_string(parameters.size()) + " arguments, but got " +
                        std::to_string(arguments.size()) + ".",
                    node->lineno, RED);
    } else {
        auto argIt = arguments.begin();
        for (size_t i = 0; i < parameters.size(); ++i, ++argIt) {
            checkExpression(*argIt, method, cls);
            std::string argumentType = inferType(*argIt, method, cls);
            if (argumentType != parameters[i].getType()) {
                reportError("Argument type mismatch for parameter " + parameters[i].getName() + ": expected " +
                                parameters[i].getType() + " but got " + argumentType,
                            (*argIt)->lineno, RED);
            }
        }
    }
}

std::string SemanticAnalyzer::getOperator(const std::string &expressionType) {
    if (expressionType == "AddExpression") return " + ";
    if (expressionType == "SubExpression") return " - ";
    if (expressionType == "MultExpression") return " * ";
    if (expressionType == "AndExpression") return " && ";
    if (expressionType == "OrExpression") return " || ";
    if (expressionType == "LTExpression") return " < ";
    if (expressionType == "GTExpression") return " > ";
    if (expressionType == "EqualExpression") return " == ";
    if (expressionType == "ArrayExpression") return "[";
    throw std::runtime_error("Couldn't get operator for expression type: " + expressionType);
}

std::string SemanticAnalyzer::getColor(const std::string &expressionType) {
    if (isArithmeticExpression(expressionType)) return BLUE;
    if (isLogicalExpression(expressionType)) return RED;
    if (isComparisonExpression(expressionType)) return RED;
    return RED;
}

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

std::string SemanticAnalyzer::inferType(Node *expression, const Method &method, const Class &cls) {
    const std::string &type = expression->type;

    static const std::unordered_map<std::string, std::string> typeMap = {
        {"IntLiteral", "Int"},       {"AddExpression", "Int"},   {"SubExpression", "Int"},
        {"MultExpression", "Int"},   {"ArrayExpression", "Int"}, {"LengthExpression", "Int"},
        {"BoolLiteral", "Bool"},     {"AndExpression", "Bool"},  {"OrExpression", "Bool"},
        {"EqualExpression", "Bool"}, {"LTExpression", "Bool"},   {"GTExpression", "Bool"},
        {"NotExpression", "Bool"},   {"IntArray", "IntArray"},   {"NewIntArrayExpression", "IntArray"}};

    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    } else if (type == "Identifier") {
        return inferIdentifierType(expression, method, cls);
    } else if (type == "MethodCallExpression") {
        Node *objectNode = expression->children.front();
        std::string objectType = inferType(objectNode, method, cls);
        if (objectType == "Int" || objectType == "Bool" || objectType == "IntArray") {
            reportError("Cannot call method on primitive type: " + objectType, expression->lineno, RED);
            return "";
        }

        if (!symbolTable.hasClass(objectType)) return "";
        const Class &objectClass = symbolTable.getClass(objectType);

        const Method &calledMethod = objectClass.getMethod(expression->value);
        return calledMethod.getReturnType();
    } else if (type == "NewObjectExpression") {
        return expression->children.front()->value;
    } else if (type == "ThisExpression") {
        return cls.getName();
    } else {
        throw std::runtime_error("Can't infer type of: " + type);
    }
}

std::string SemanticAnalyzer::inferIdentifierType(Node *expression, const Method &method, const Class &cls) {
    std::string varName = expression->value;

    std::string varType = symbolTable.getVariableType(varName, method.getName(), cls.getName());
    if (!varType.empty()) {
        return varType;
    }

    reportError("Variable '" + varName + "' is not declared in the method or class scope.", expression->lineno, RESET);
    return "";
}

void SemanticAnalyzer::reportError(const std::string &message, int lineno, const std::string &color) {
    std::cerr << color << "\@error at line " << lineno << ": " << message << RESET << std::endl;
    semanticErrors++;
}