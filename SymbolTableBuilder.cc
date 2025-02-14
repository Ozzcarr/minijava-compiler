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
                if (methodBodyChild->type == "VarDeclarationList") {
                    for (auto varChild : methodBodyChild->children) {
                        if (varChild->type == "VarDeclaration" && varChild->children.size() >= 2) {
                            auto typeIt = varChild->children.begin();
                            auto nameIt = std::next(typeIt);
                            Variable var((*nameIt)->value, (*typeIt)->value);
                            method.addParameter(var);
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

void buildSymbolTable(Node *root, SymbolTable &symbolTable) {
    if (!root) return;

    std::stack<Node *> nodeStack;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
        Node *current = nodeStack.top();
        nodeStack.pop();

        if (current->type == "MainClass") {
            Class cls(current->value);
            std::cout << "MainClass: " << cls.getName() << std::endl;
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