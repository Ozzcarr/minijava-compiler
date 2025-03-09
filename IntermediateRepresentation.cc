#include "IntermediateRepresentation.h"


int BasicBlock::tempCounter = 0;

void ControlFlowGraph::writeCFG() {
    // Print the block names
    for (size_t i = 0; i < blocks.size(); i++) {
        std::cout << "Block " << i << ": " << blocks[i]->name << std::endl;
    }

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

        outFile << i << " [label=\"" << blocks[i]->name << "\\n" << std::endl;

        // Add instructions to the block
        for (const auto &instruction : block->getTacInstructions()) {
            outFile << "    ";
            if (instruction.op == "print" || instruction.op == "param" || instruction.op == "if" || instruction.op == "return") {
                outFile << instruction.op << " " << instruction.arg1 << std::endl;
            } else if (instruction.op == "call" || instruction.op == "new") {
                outFile << instruction.result << " := " << instruction.op << " " << instruction.arg1 << " "
                        << instruction.arg2 << std::endl;
            } else if (instruction.op.empty()) {  // No operation
                outFile << instruction.result << " := " << instruction.arg1 << std::endl;
            } else if (instruction.arg2.empty()) {  // Unary operation
                std::string endOp = (instruction.op == "new int[") ? "]" : "";
                outFile << instruction.result << " := " << instruction.op << instruction.arg1 << endOp << std::endl;
            } else {  // Binary operation
                std::string endOp = (instruction.op == "[") ? "]" : "";
                outFile << instruction.result << " := " << instruction.arg1 << instruction.op << instruction.arg2
                        << endOp << std::endl;
            }
        }
        outFile << "\"];" << std::endl;

        // Add connections to the block
        if (block->trueExit) {
            // Find the index of the true exit block
            size_t trueExitIndex =
                std::distance(blocks.begin(), std::find(blocks.begin(), blocks.end(), block->trueExit));
            outFile << i << " -> " << trueExitIndex << " [xlabel=\"true\"];" << std::endl;
        }
        if (block->falseExit) {
            size_t falseExitIndex =
                std::distance(blocks.begin(), std::find(blocks.begin(), blocks.end(), block->falseExit));
            outFile << i << " -> " << falseExitIndex << " [xlabel=\"false\"];" << std::endl;
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
    traverseClassDeclarationList(classDeclListNode);
}

void ControlFlowGraph::traverseMainClass(Node *node) {
    if (!node) throw std::runtime_error("Main class node is null");
    if (node->type != "MainClass") throw std::runtime_error("Invalid node type for main class: " + node->type);

    currentClassName = node->value;

    Node *statementListNode = findChild(node, "StatementList");
    if (!statementListNode) throw std::runtime_error("No statement list found in main class");

    std::string entryName = currentClassName + ".main";
    BasicBlock *block = new BasicBlock(entryName);
    BasicBlock *currentBlock = block;

    for (auto child : statementListNode->children) {
        if (endsWith(child->type, "Statement")) {
            currentBlock = traverseStatement(child, currentBlock);
        } else {
            throw std::runtime_error("Unknown child type in main class statement list: " + child->type);
        }
    }

    blocks.emplace_back(block);
}

void ControlFlowGraph::traverseClassDeclarationList(Node *node) {
    if (!node) throw std::runtime_error("Class declaration list node is null");
    if (node->type != "ClassDeclarationList")
        throw std::runtime_error("Invalid node type for class declaration list: " + node->type);

    for (auto child : node->children) {
        if (child->type == "ClassDeclaration") {
            traverseClassDeclaration(child);
        } else {
            throw std::runtime_error("Unknown child type in class declaration list: " + child->type);
        }
    }
}

void ControlFlowGraph::traverseClassDeclaration(Node *node) {
    if (!node) throw std::runtime_error("Class declaration node is null");
    if (node->type != "ClassDeclaration")
        throw std::runtime_error("Invalid node type for class declaration: " + node->type);

    currentClassName = node->value;

    Node *methodDeclListNode = findChild(node, "MethodDeclarationList");
    if (!methodDeclListNode) throw std::runtime_error("No method declaration list found in class declaration");

    for (auto child : methodDeclListNode->children) {
        if (child->type == "MethodDeclaration") {
            traverseMethodDeclaration(child);
        } else {
            throw std::runtime_error("Unknown child type in method declaration list: " + child->type);
        }
    }
}

void ControlFlowGraph::traverseMethodDeclaration(Node *node) {
    if (!node) throw std::runtime_error("Method declaration node is null");
    if (node->type != "MethodDeclaration")
        throw std::runtime_error("Invalid node type for method declaration: " + node->type);

    Node *code = findChild(node, "Code");
    if (!code) throw std::runtime_error("No code found in method declaration");

    std::string entryName = currentClassName + "." + node->value;
    BasicBlock *entryBlock = new BasicBlock(entryName);
    blocks.emplace_back(entryBlock);
    size_t nextBlockIndex = blocks.size();

    BasicBlock *lastBlock = traverseCode(code, entryBlock);
    if (lastBlock != entryBlock) entryBlock->trueExit = blocks[nextBlockIndex];

    // Add a return instruction to the last block
    Node *returnNode = findChild(node, "Return");
    if (!returnNode) throw std::runtime_error("No return found in method declaration");
    if (returnNode->children.size() != 1) throw std::runtime_error("Invalid number of children for return");
    std::string returnValue = traverseExpression(returnNode->children.front(), lastBlock);
    lastBlock->addInstruction("return", returnValue);
}

BasicBlock *ControlFlowGraph::traverseCode(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("Code node is null");
    if (node->type != "Code") throw std::runtime_error("Invalid node type for code: " + node->type);

    BasicBlock *currentBlock = block;
    for (auto child : node->children) {
        if (child->type == "Variable") {
            continue;  // Skip variable declarations
        } else if (endsWith(child->type, "Statement")) {
            currentBlock = traverseStatement(child, currentBlock);
        } else {
            throw std::runtime_error("Unknown child type in code: " + child->type);
        }
    }

    return currentBlock;  // Return the last block processed
}

BasicBlock *ControlFlowGraph::traverseStatement(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("Statement node is null");

    std::string statementType = node->type;
    BasicBlock *resultBlock = block;

    if (statementType == "PrintStatement") {
        resultBlock = traversePrintStatement(node, block);
    } else if (statementType == "WhileStatement") {
        resultBlock = traverseWhileStatement(node, block);
    } else if (statementType == "IfStatement") {
        resultBlock = traverseIfStatement(node, block);
    } else if (statementType == "IfElseStatement") {
        resultBlock = traverseIfElseStatement(node, block);
    } else if (statementType == "ArrayInitStatement") {
        std::string varName = node->children.front()->value;
        if (node->children.size() != 3) throw std::runtime_error("Invalid number of children for array init statement");
        auto it = node->children.begin();
        Node *sizeNode = (*(++it));
        std::string size = traverseExpression(sizeNode, block);
        std::string newVarName = varName + "[" + size + "]";

        std::string expression = traverseExpression(node->children.back(), block);
        block->addInstruction(newVarName, "", expression);
    } else if (statementType == "VarInitStatement") {
        std::string varName = node->children.front()->value;
        std::string value = traverseExpression(node->children.back(), block);
        block->addInstruction(varName, "", value);
    } else {
        throw std::runtime_error("Unknown statement type: " + statementType);
    }

    tempCounter = 0;
    return resultBlock;
}

BasicBlock *ControlFlowGraph::traversePrintStatement(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("Print statement node is null");
    if (node->type != "PrintStatement")
        throw std::runtime_error("Invalid node type for print statement: " + node->type);
    if (node->children.size() != 1) throw std::runtime_error("Invalid number of children for print statement");

    Node *expressionNode = node->children.front();
    if (!expressionNode) throw std::runtime_error("No expression found in print statement");

    std::string varName = traverseExpression(expressionNode, block);
    block->addInstruction("print", varName);

    return block;
}

BasicBlock *ControlFlowGraph::traverseWhileStatement(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("While statement node is null");
    if (node->type != "WhileStatement")
        throw std::runtime_error("Invalid node type for while statement: " + node->type);
    if (node->children.size() != 2) throw std::runtime_error("Invalid number of children for while statement");

    Node *conditionNode = node->children.front();
    Node *bodyNode = node->children.back();
    if (!conditionNode || !bodyNode) throw std::runtime_error("Invalid children for while statement");

    // Create blocks for the while statement
    BasicBlock *conditionBlock = new BasicBlock();
    BasicBlock *bodyBlock = new BasicBlock();
    BasicBlock *exitBlock = new BasicBlock();

    // Process condition
    std::string conditionVar = traverseExpression(conditionNode, conditionBlock);
    conditionBlock->addInstruction("if", conditionVar);

    blocks.emplace_back(conditionBlock);
    blocks.emplace_back(bodyBlock);
    blocks.emplace_back(exitBlock);

    // Connect blocks
    block->trueExit = conditionBlock;
    conditionBlock->trueExit = bodyBlock;
    conditionBlock->falseExit = exitBlock;

    // Process body
    BasicBlock *currentBlock = bodyBlock;
    for (auto child : bodyNode->children) {
        currentBlock = traverseStatement(child, currentBlock);
    }

    if (!currentBlock->trueExit && !currentBlock->falseExit) {
        currentBlock->trueExit = conditionBlock;
    }

    return exitBlock;
}

BasicBlock *ControlFlowGraph::traverseIfStatement(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("If statement node is null");
    if (node->type != "IfStatement")
        throw std::runtime_error("Invalid node type for if statement: " + node->type);
    if (node->children.size() != 2) throw std::runtime_error("Invalid number of children for if statement");

    Node *conditionNode = node->children.front();
    Node *ifBodyNode = findChild(node, "StatementList");
    if (!conditionNode || !ifBodyNode) throw std::runtime_error("Invalid children for if statement");

    // Create blocks for the if statement
    BasicBlock *conditionBlock = new BasicBlock();
    BasicBlock *ifBodyBlock = new BasicBlock();
    BasicBlock *exitBlock = new BasicBlock();

    // Connect the current block to the condition block
    block->trueExit = conditionBlock;

    // Process condition
    std::string conditionVar = traverseExpression(conditionNode->children.front(), conditionBlock);
    conditionBlock->addInstruction("if", conditionVar);

    // Add all blocks to the blocks vector BEFORE processing body
    blocks.emplace_back(conditionBlock);
    blocks.emplace_back(ifBodyBlock);
    blocks.emplace_back(exitBlock);

    // Connect condition block to if body or exit
    conditionBlock->trueExit = ifBodyBlock;
    conditionBlock->falseExit = exitBlock;

    // Process if body
    BasicBlock *ifCurrentBlock = ifBodyBlock;
    for (auto child : ifBodyNode->children) {
        if (endsWith(child->type, "Statement")) {
            ifCurrentBlock = traverseStatement(child, ifCurrentBlock);
        } else {
            throw std::runtime_error("Unknown child type in if body: " + child->type);
        }
    }

    // Connect if body to exit if it doesn't already have an exit
    if (!ifCurrentBlock->trueExit && !ifCurrentBlock->falseExit) {
        ifCurrentBlock->trueExit = exitBlock;
    }

    return exitBlock;
}

BasicBlock *ControlFlowGraph::traverseIfElseStatement(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("If else statement node is null");
    if (node->type != "IfElseStatement")
        throw std::runtime_error("Invalid node type for if else statement: " + node->type);
    if (node->children.size() != 3) throw std::runtime_error("Invalid number of children for if else statement");

    Node *conditionNode = node->children.front();
    Node *ifBodyNode = findChild(node, "StatementList");
    Node *elseBodyNode = findChild(node, "StatementList", 2);
    if (!conditionNode || !ifBodyNode || !elseBodyNode) throw std::runtime_error("Invalid children for if else statement");

    // Create blocks for the if-else statement
    BasicBlock *conditionBlock = new BasicBlock();
    BasicBlock *ifBodyBlock = new BasicBlock();
    BasicBlock *elseBodyBlock = new BasicBlock();
    BasicBlock *exitBlock = new BasicBlock();

    block->trueExit = conditionBlock;

    // Process condition
    std::string conditionVar = traverseExpression(conditionNode->children.front(), conditionBlock);
    conditionBlock->addInstruction("if", conditionVar);

    // Add all blocks to the blocks vector
    blocks.emplace_back(conditionBlock);
    blocks.emplace_back(ifBodyBlock);
    blocks.emplace_back(elseBodyBlock);
    blocks.emplace_back(exitBlock);

    // Process if body
    BasicBlock *ifCurrentBlock = ifBodyBlock;
    for (auto child : ifBodyNode->children) {
        if (endsWith(child->type, "Statement")) {
            ifCurrentBlock = traverseStatement(child, ifCurrentBlock);
        } else {
            throw std::runtime_error("Unknown child type in if body: " + child->type);
        }
    }

    // Process else body
    BasicBlock *elseCurrentBlock = elseBodyBlock;
    for (auto child : elseBodyNode->children) {
        if (endsWith(child->type, "Statement")) {
            elseCurrentBlock = traverseStatement(child, elseCurrentBlock);
        } else {
            throw std::runtime_error("Unknown child type in else body: " + child->type);
        }
    }

    // Connect blocks
    ifCurrentBlock->trueExit = exitBlock;
    elseCurrentBlock->trueExit = exitBlock;
    conditionBlock->trueExit = ifBodyBlock;
    conditionBlock->falseExit = elseBodyBlock;

    return exitBlock;
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
    } else if (expressionType == "MethodCallExpression") {
        return traverseMethodCall(node, block);
    } else if (expressionType == "NewObjectExpression") {
        std::string varName = generateName();
        block->addInstruction(varName, "new", traverseExpression(node->children.front(), block));
        block->addInstruction("param", varName);
        return varName;
    } else if (expressionType == "Identifier") {
        return node->value;
    } else if (expressionType == "ThisExpression") {
        std::string varName = generateName();
        block->addInstruction(varName, "", currentClassName);
        block->addInstruction("param", varName);
        return varName;
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
    block->addInstruction(varName, getOperator(node->type), childName);
    return varName;
}

std::string ControlFlowGraph::traverseMethodCall(Node *node, BasicBlock *block) {
    if (!node) throw std::runtime_error("Method call node is null");
    if (node->children.size() != 2) throw std::runtime_error("Invalid number of children for method call");

    std::string varName = generateName();

    std::string className = traverseExpression(node->children.front(), block);

    Node *callOnNode = node->children.front();
    Node *argsNode = node->children.back();
    if (!callOnNode || !argsNode) throw std::runtime_error("Invalid children for method call");

    std::string methodName = node->value;
    for (auto arg : argsNode->children) {
        std::string argName = traverseExpression(arg, block);
        block->addInstruction("param", argName);
    }

    std::string callVarName = currentClassName + "." + methodName;
    block->addInstruction(varName, callVarName, "call", std::to_string(argsNode->children.size() + 1));
    return varName;
}
