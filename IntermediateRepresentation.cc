#include "IntermediateRepresentation.h"

void ControlFlowGraph::writeCFG() {
    std::ofstream outFile("cfg.dot");
    if (!outFile) {
        std::cerr << "Error opening file for writing: cfg.dot" << std::endl;
        return;
    }

    outFile << "digraph G {" << std::endl;
    outFile << "graph [splines=ortho];" << std::endl;
    outFile << "node [shape=box];" << std::endl;

    for (size_t i = 0; i < blocks.size(); i++) {
        const auto &block = blocks[i];

        outFile << i << " [label=\"" << block.name << "\\n" << std::endl;

        // Add instructions to the block
        for (const auto &instr : block.getTacInstructions()) {
            outFile << "    ";
            if (instr.op == "print") {
                outFile << instr.op << " _t0" << std::endl;
            } else if (instr.arg2.empty()) {  // Unary operation
                outFile << instr.result << " := " << instr.op << instr.arg1 << std::endl;
            } else {  // Binary operation
                outFile << instr.result << " := " << instr.arg1 << instr.op << instr.arg2 << std::endl;
            }
        }
        outFile << "\"];" << std::endl;

        // Add connections to the block
        if (block.trueExit != -1) {
            outFile << i << " -> " << block.trueExit << " [xlabel=\"true\"];" << std::endl;
        }
        if (block.falseExit != -1) {
            outFile << i << " -> " << block.falseExit << " [xlabel=\"false\"];" << std::endl;
        }
    }

    outFile << "}" << std::endl;
    outFile.close();
}

void ControlFlowGraph::traverseAST(Node *root) {
    if (!root) return;
    if (root->type != "Goal") throw std::runtime_error("Invalid root node type: " + root->type);
    if (root->children.size() != 2) throw std::runtime_error("Invalid number of children for root node");

    Node *mainClassNode = root->children.front();
    Node *classDeclListNode = root->children.back();
    if (!mainClassNode || !classDeclListNode) throw std::runtime_error("Invalid children for root node");

    traverseMainClass(mainClassNode);
    // traverseClassDeclList(classDeclListNode);
}

void ControlFlowGraph::traverseMainClass(Node *node) {
    if (!node) throw std::runtime_error("Main class node is null");
    if (node->type != "MainClass") throw std::runtime_error("Invalid node type for main class: " + node->type);

    Node *statementListNode = findChild(node, "StatementList");
    if (!statementListNode) throw std::runtime_error("No statement list found in main class");

    traverseStatementList(statementListNode);
}

void ControlFlowGraph::traverseStatementList(Node *node) {
    if (!node) throw std::runtime_error("Statement list node is null");
    if (node->type != "StatementList") throw std::runtime_error("Invalid node type for statement list: " + node->type);

    // Keep track of the previous block to connect sequential statements
    int prevBlockIndex = -1;

    for (auto child : node->children) {
        int currentBlockIndex = blocks.size();
        traverseStatement(child);

        // Connect sequential blocks if there was a previous block
        if (prevBlockIndex >= 0 && blocks[prevBlockIndex].trueExit == -1) {
            blocks[prevBlockIndex].trueExit = currentBlockIndex;
        }

        prevBlockIndex = currentBlockIndex;
    }
}

void ControlFlowGraph::traverseStatement(Node *node) {
    if (!node) throw std::runtime_error("Statement node is null");

    std::string statementType = node->type;

    if (statementType == "PrintStatement") {
        traversePrintStatement(node);
    } else {
        throw std::runtime_error("Unknown statement type: " + statementType);
    }

    tempCounter = 0;
}

void ControlFlowGraph::traversePrintStatement(Node *node) {
    if (!node) throw std::runtime_error("Print statement node is null");
    if (node->type != "PrintStatement")
        throw std::runtime_error("Invalid node type for print statement: " + node->type);
    if (node->children.size() != 1) throw std::runtime_error("Invalid number of children for print statement");

    Node *expressionNode = node->children.front();
    if (!expressionNode) throw std::runtime_error("No expression found in print statement");

    BasicBlock block("PrintStatement");
    traverseExpression(expressionNode, &block);
    block.addInstruction("print", expressionNode->value, "print", "");
    blocks.push_back(block);
}

std::string ControlFlowGraph::traverseExpression(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("Expression node is null");

    std::string expressionType = node->type;

    if (isBinaryExpression(expressionType)) {
        return traverseBinaryExpression(node, block);
    } else if (isUnaryExpression(expressionType)) {
        return traverseUnaryExpression(node, block);
    } else if (isLiteral(expressionType)) {
        return node->value;
    } else {
        throw std::runtime_error("Unknown expression type: " + expressionType);
    }
}

std::string ControlFlowGraph::traverseBinaryExpression(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("Binary expression node is null");
    if (node->children.size() != 2) throw std::runtime_error("Invalid number of children for binary expession");

    std::string varName = generateName();

    Node *leftChild = node->children.front();
    Node *rightChild = node->children.back();
    if (!leftChild || !rightChild) throw std::runtime_error("Invalid children for binary expression");

    std::string leftName = traverseExpression(leftChild, block);
    std::string rightName = traverseExpression(rightChild, block);

    block->addInstruction(varName, leftName, getOperator(node->type), rightName);
    return varName;
}

std::string ControlFlowGraph::traverseUnaryExpression(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("Unary expression node is null");
    if (node->children.size() != 1) throw std::runtime_error("Invalid number of children for unary expression");

    std::string varName = generateName();

    Node *childNode = node->children.front();
    if (!childNode) throw std::runtime_error("No child found for unary expression");

    std::string childName = traverseExpression(childNode, block);
    block->addInstruction(varName, childName, getOperator(node->type), "");
    return varName;
}