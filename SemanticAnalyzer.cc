#include "SemanticAnalyzer.h"

// Main analysis functions

void SemanticAnalyzer::analyze(Node *root) {
    if (!root) throw std::runtime_error("Root node is null.");
    std::vector<std::string> classNames;

    // Check main class
    Node *mainClassNode = findChild(root, "MainClass");
    if (mainClassNode) {
        classNames.push_back(mainClassNode->value);
        Node *statementList = findChild(mainClassNode, "StatementList");
        if (!statementList) throw std::runtime_error("No statement list found in main class.");
        Class mainClass = symbolTable.getClass(mainClassNode->value);
        for (auto child : statementList->children) {
            checkStatement(child, Method("main", "void"), mainClass);
        }
    } else {
        throw std::runtime_error("No main class found in the AST.");
    }

    // Check class declarations
    Node *classDeclList = findChild(root, "ClassDeclarationList");
    if (classDeclList) {
        for (auto child : classDeclList->children) {
            if (child->type == "ClassDeclaration") {
                // Check for duplicate class names
                if (std::find(classNames.begin(), classNames.end(), child->value) != classNames.end()) {
                    reportError("Class " + child->value + " is declared multiple times.", child->lineno, PURPLE);
                }
                classNames.push_back(child->value);

                checkClass(child, classNames);
            }
        }
    } else {
        throw std::runtime_error("No class declaration list found in the AST.");
    }
}

void SemanticAnalyzer::checkClass(Node *node, std::vector<std::string> &clsNames) {
    std::string className = node->value;

    if (!symbolTable.hasClass(className)) {
        reportError("Class " + className + " is not declared.", node->lineno, RESET);
        return;
    }

    // Check that class variables are of existing types
    Node *varDeclList = findChild(node, "VarDeclarationList");
    std::vector<std::string> classVars;
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

            // Check for duplicate variable names
            if (std::find(classVars.begin(), classVars.end(), varName) != classVars.end()) {
                reportError("Class variable " + varName + " is declared multiple times in class " + className,
                            varLineno, PURPLE);
            }
            classVars.push_back(varName);
        }
    }

    int occurence = std::count(clsNames.begin(), clsNames.end(), className);
    if (occurence == 0) reportError("Class " + className + " is not declared.", node->lineno, RESET);
    Class cls = symbolTable.getOccurenceOfClass(className, occurence);

    Node *methodDeclList = findChild(node, "MethodDeclarationList");

    if (!methodDeclList) throw std::runtime_error("No method declaration list found in class " + className);
    std::vector<std::string> methodNames;

    for (auto child : methodDeclList->children) {
        if (child->type == "MethodDeclaration") {
            if (std::find(methodNames.begin(), methodNames.end(), child->value) != methodNames.end()) {
                reportError("Method " + child->value + " is declared multiple times in class " + className,
                            child->lineno, PURPLE);
            }
            methodNames.push_back(child->value);

            checkMethod(child, cls);
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

    // Check method parameters for duplicate names
    std::vector<std::string> paramNames;
    for (const auto &param : method.getParameters()) {
        if (std::find(paramNames.begin(), paramNames.end(), param.getName()) != paramNames.end()) {
            reportError("Method parameter " + param.getName() + " is declared multiple times in method " + methodName,
                        node->lineno, PURPLE);
        }
        paramNames.push_back(param.getName());
    }

    // Check local variables for duplicate names
    std::vector<std::string> localVars;
    auto localVariables = method.getLocalVariables();
    for (const auto &localVar : localVariables) {
        if (std::find(localVars.begin(), localVars.end(), localVar.first.getName()) != localVars.end()) {
            reportError(
                "Local variable " + localVar.first.getName() + " is declared multiple times in method " + methodName,
                localVar.second, PURPLE);
        }
        if (std::find(paramNames.begin(), paramNames.end(), localVar.first.getName()) != paramNames.end()) {
            reportError("Local variable " + localVar.first.getName() + " has the same name as a parameter in method " +
                            methodName,
                        localVar.second, PURPLE);
        }
        localVars.push_back(localVar.first.getName());
    }

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
        checkVarInitStatement(node, method, cls);
    } else if (statementType == "ArrayInit") {
        checkArrayInitStatement(node, method, cls);
    } else if (statementType == "If") {
        checkIfStatement(node, method, cls);
    } else if (statementType == "IfElse") {
        checkIfElseStatement(node, method, cls);
    } else if (statementType == "While") {
        checkWhileStatement(node, method, cls);
    } else if (statementType == "Print") {
        checkPrintStatement(node, method, cls);
    } else {
        throw std::runtime_error("Unknown statement type: " + statementType + " on line " +
                                 std::to_string(node->lineno));
    }
}

void SemanticAnalyzer::checkVarInitStatement(Node *node, const Method &method, const Class &cls) {
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

    if (method.isLocalVariable(varName)) {
        if (!method.isVariableDeclaredBefore(varName, node->lineno)) {
            reportError("Variable '" + varName + "' is used before it is declared.", node->lineno, RED);
        }
    }
}

void SemanticAnalyzer::checkArrayInitStatement(Node *node, const Method &method, const Class &cls) {
    if (node->children.size() != 3) throw std::runtime_error("Array initialization must have exactly 3 children");
    auto it = node->children.begin();
    Node *var = *it;
    Node *size = *(++it);
    Node *expression = *(++it);

    std::string varName = var->value;
    std::string varType = inferType(var, method, cls);
    if (varType != "IntArray") {
        reportError("Array initialization mismatch: variable '" + varName + "' is declared as " + varType +
                        " but assigned IntArray",
                    node->lineno, YELLOW);
    }

    checkExpression(size, method, cls);
    std::string sizeType = inferType(size, method, cls);
    if (sizeType != "Int") {
        reportError("Array size must be of type Int, but got " + sizeType, node->lineno, RED);
    }

    checkExpression(expression, method, cls);
    std::string expressionType = inferType(expression, method, cls);
    if (expressionType != "Int") {
        reportError("Array initialization mismatch: variable '" + varName + "' is declared as " + varType +
                        " but assigned " + expressionType,
                    node->lineno, YELLOW);
    }
}

void SemanticAnalyzer::checkIfStatement(Node *node, const Method &method, const Class &cls) {
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

    Node *statementList = findChild(node, "StatementList");
    if (!statementList) throw std::runtime_error("No statement list found in if statement");

    for (auto child : statementList->children) {
        checkStatement(child, method, cls);
    }
}

void SemanticAnalyzer::checkIfElseStatement(Node *node, const Method &method, const Class &cls) {
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
}

void SemanticAnalyzer::checkWhileStatement(Node *node, const Method &method, const Class &cls) {
    if (node->children.size() != 2) throw std::runtime_error("While statement must have exactly two children");
    Node *condition = node->children.front();
    if (!condition) throw std::runtime_error("No condition found in while statement");
    checkExpression(condition, method, cls);
    std::string conditionType = inferType(condition, method, cls);
    if (conditionType != "Bool") {
        reportError("Condition must be of type Bool, but got " + conditionType, node->lineno, RED);
    }

    Node *statementList = node->children.back();
    if (!statementList) throw std::runtime_error("No statement list found in while statement");
    for (auto child : statementList->children) {
        checkStatement(child, method, cls);
    }
}

void SemanticAnalyzer::checkPrintStatement(Node *node, const Method &method, const Class &cls) {
    if (node->children.size() != 1) throw std::runtime_error("Print statement must have exactly one expression");
    Node *expression = node->children.front();
    checkExpression(expression, method, cls);
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
    } else if (expressionType != "ArgumentList" && expressionType != "IntLiteral" && expressionType != "BoolLiteral" &&
               expressionType != "Identifier" && expressionType != "ThisExpression") {
        throw std::runtime_error("Unknown expression type: " + expressionType);
    }
}

// Helper functions

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