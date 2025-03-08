#ifndef INTERMEDIATE_REPRESENTATION_H
#define INTERMEDIATE_REPRESENTATION_H

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "HelperFunctions.h"
#include "Node.h"

class BasicBlock {
   private:
    struct ThreeAdressCode {
        std::string result;
        std::string arg1;
        std::string op;
        std::string arg2;

        ThreeAdressCode(const std::string &result, const std::string &arg1, const std::string &op,
                        const std::string &arg2)
            : result(result), arg1(arg1), op(op), arg2(arg2) {}
    };

   public:
    std::string name;
    BasicBlock *trueExit;
    BasicBlock *falseExit;

    BasicBlock(const std::string &name) : name(name), trueExit(nullptr), falseExit(nullptr) {}

    inline void addInstruction(const std::string &op, const std::string &arg1) {
        tacInstructions.emplace_back("", arg1, op, "");
    }

    inline void addInstruction(const std::string &result, const std::string &op, const std::string &arg1) {
        tacInstructions.emplace_back(result, arg1, op, "");
    }

    inline void addInstruction(const std::string &result, const std::string &arg1, const std::string &op,
                               const std::string &arg2) {
        tacInstructions.emplace_back(result, arg1, op, arg2);
    }

    std::vector<ThreeAdressCode> getTacInstructions() const { return tacInstructions; }

   private:
    int tempCounter = 0;
    std::vector<ThreeAdressCode> tacInstructions;
};

class ControlFlowGraph {
   public:
    /**
     * @brief Writes the control flow graph to a dot file.
     */
    void writeCFG();

    /**
     * @brief Traverses the AST and generates the control flow graph.
     */
    void traverseAST(Node *root);

   private:
    std::vector<BasicBlock *> blocks;
    int tempCounter = 0;

    std::string generateName() { return "_t" + std::to_string(tempCounter++); }

    void traverseMainClass(Node *node);
    void traverseClassDeclarationList(Node *node);
    void traverseClassDeclaration(Node *node);
    void traverseMethodDeclaration(Node *node, const std::string &className);
    BasicBlock* traverseCode(Node *node, BasicBlock *block);
    BasicBlock *traverseStatement(Node *node, BasicBlock *block);
    BasicBlock *traversePrintStatement(Node *node, BasicBlock *block);
    BasicBlock *traverseWhileStatement(Node *node, BasicBlock *block);
    BasicBlock *traverseIfStatement(Node *node, BasicBlock *block);
    BasicBlock *traverseIfElseStatement(Node *node, BasicBlock *block);
    std::string traverseExpression(Node *node, BasicBlock *block);
    std::string traverseUnaryExpression(Node *node, BasicBlock *block);
    std::string traverseBinaryExpression(Node *node, BasicBlock *block);
    std::string traverseMethodCall(Node *node, BasicBlock *block);
};

#endif  // INTERMEDIATE_REPRESENTATION_H
