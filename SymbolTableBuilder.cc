#include "SymbolTableBuilder.h"

#include <iostream>
#include <stack>

void processVarDeclarationList(Node *child, Class &cls) {
    for (auto varChild : child->children) {
        if (varChild->type == "VarDeclaration" && varChild->children.size() >= 2) {
            auto typeIt = varChild->children.begin();
            auto nameIt = std::next(typeIt);
            Variable var((*nameIt)->value, (*typeIt)->value);
            cls.addVariable(var);
        }
    }
}

void processMethodDeclarationList(Node *child, Class &cls) {
    for (auto methodChild : child->children) {
        if (methodChild->type == "MethodDeclaration" && methodChild->children.size() >= 1) {
            auto returnType = methodChild->children.begin();
            Method method(methodChild->value, (*returnType)->value);

            auto paramList = std::next(methodChild->children.begin());
            if (paramList != methodChild->children.end() && (*paramList)->type == "ParameterList") {
                for (auto param : (*paramList)->children) {
                    if (param->children.size() >= 1) {
                        auto paramType = param->children.begin();
                        method.addParameter(Variable(param->value, (*paramType)->value));
                    }
                }
            }

            for (auto methodBodyChild : methodChild->children) {
                if (methodBodyChild->type == "Code") {
                    for (auto varChild : methodBodyChild->children) {
                        if (varChild->type == "Variable") {
                            std::string name = varChild->value;
                            std::string type = (*varChild->children.begin())->value;
                            Variable var(name, type);
                            int lineNumber = varChild->lineno;
                            method.addLocalVariable(var, lineNumber);
                        }
                    }
                }
            }
            cls.addMethod(method);
        }
    }
}

void processClassDeclaration(Node *current, SymbolTable &symbolTable) {
    Class cls(current->value);

    for (auto child : current->children) {
        if (child->type == "VarDeclarationList") {
            processVarDeclarationList(child, cls);
        } else if (child->type == "MethodDeclarationList") {
            processMethodDeclarationList(child, cls);
        }
    }

    symbolTable.addClass(cls);
}

void processMainClass(Node *current, SymbolTable &symbolTable) {
    Method mainMethod("main", "void");
    if (current->children.size() != 2) throw std::runtime_error("Invalid main declaration");
    // mainMethod.addParameter(Variable(current->children.front()->value, "StringArgs"));

    Class cls(current->value);
    cls.addMethod(mainMethod);
    symbolTable.addClass(cls);
}

void buildSymbolTable(Node *root, SymbolTable &symbolTable) {
    if (!root) return;

    std::stack<Node *> nodeStack;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
        Node *current = nodeStack.top();
        nodeStack.pop();

        if (current->type == "MainClass") {
            processMainClass(current, symbolTable);
        } else if (current->type == "ClassDeclaration") {
            processClassDeclaration(current, symbolTable);
        }

        for (auto child : current->children) {
            if (child) {
                nodeStack.push(child);
            }
        }
    }
}