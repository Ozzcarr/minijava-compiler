#include "BytecodeGenerator.h"

// Helper function to normalize boolean values
std::string normalizeBooleanValue(const std::string& value) {
    if (value == "true") return "1";
    if (value == "false") return "0";
    return value;
}

// Get the corresponding opcode for an operation
OpCode getOpCodeForOperation(const std::string& op, std::string& arg1, std::string& arg2) {
    if (op == " + ") return OpCode::IADD;
    if (op == " - ") return OpCode::ISUB;
    if (op == " * ") return OpCode::IMUL;
    if (op == " < ") return OpCode::ILT;
    if (op == " > ") return OpCode::IGT;
    if (op == " == ") return OpCode::IEQ;

    if (op == " && " || op == " || ") {
        arg1 = normalizeBooleanValue(arg1);
        arg2 = normalizeBooleanValue(arg2);
        return (op == " && ") ? OpCode::IAND : OpCode::IOR;
    }

    throw std::runtime_error("Unknown operation: " + op);
}

void BCProgram::generateBytecode(const ControlFlowGraph& cfg, const SymbolTable& symbolTable) {
    TypeTracker typeTracker(symbolTable);

    // Process each basic block in the CFG
    for (const auto& block : cfg.getBlocks()) {
        auto bytecodeBlock = std::make_unique<BCBlock>(block->name);
        bool stop = true;

        // Extract class and method names
        std::string className = block->name.substr(0, block->name.find('.'));
        std::string methodName = block->name.substr(block->name.find('.') + 1);
        bool isMainMethod = methodName == "main";

        // Store method parameters
        if (symbolTable.hasClass(className)) {
            Class cls = symbolTable.getClass(className);
            if (cls.hasMethod(methodName)) {
                for (const auto& param : cls.getMethod(methodName).getParameters()) {
                    bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, param.getName()));
                }
            }
        }

        // Helper function for loading values
        auto addLoadInstruction = [&](const std::string& arg) {
            OpCode opType = (arg.find_first_not_of("0123456789") == std::string::npos) ? OpCode::ICONST : OpCode::ILOAD;
            bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(opType, arg));
        };

        // First pass - identify variable types
        for (const auto& tacInst : block->getTacInstructions()) {
            if (tacInst.op.empty()) {
                typeTracker.trackAssignment(tacInst.result, tacInst.arg1);
            } else if (tacInst.op == "new") {
                typeTracker.trackNewObject(tacInst.result, tacInst.arg1);
            }
        }

        // Second pass - generate bytecode
        std::vector<std::string> pendingParams;

        for (const auto& tacInst : block->getTacInstructions()) {
            if (tacInst.op == "param") {
                pendingParams.push_back(tacInst.arg1);
            } else if (tacInst.op == "print") {
                addLoadInstruction(tacInst.arg1);
                bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::PRINT));
            } else if (tacInst.op == "return") {
                addLoadInstruction(tacInst.arg1);
                bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::IRETURN));
                stop = false;
            } else if (tacInst.op == " + " || tacInst.op == " - " || tacInst.op == " * " || tacInst.op == " < " ||
                       tacInst.op == " > " || tacInst.op == " == " || tacInst.op == " && " || tacInst.op == " || ") {
                // Handle binary operations
                std::string arg1 = tacInst.arg1;
                std::string arg2 = tacInst.arg2;
                OpCode op = getOpCodeForOperation(tacInst.op, arg1, arg2);

                addLoadInstruction(arg1);
                addLoadInstruction(arg2);
                bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(op));
                bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
            } else if (tacInst.op == "!") {
                // Unary NOT operation
                std::string arg1 = normalizeBooleanValue(tacInst.arg1);
                addLoadInstruction(arg1);
                bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::INOT));
                bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
            } else if (tacInst.op == "if") {
                addLoadInstruction(tacInst.arg1);
                bytecodeBlock->addInstruction(
                    std::make_unique<BCInstruction>(OpCode::IFFALSEGOTO, block->falseExit->name));
            } else if (tacInst.op == "call") {
                // Process method call
                std::string methodToCall = tacInst.arg1;

                if (!pendingParams.empty()) {
                    // Get the class reference and resolve its type
                    std::string classRef = pendingParams[0];
                    std::string actualClassName = typeTracker.resolveClassName(classRef);

                    // Add all parameters except the first (class reference)
                    for (int i = pendingParams.size() - 1; i > 0; i--) {
                        addLoadInstruction(pendingParams[i]);
                    }

                    // Form the fully qualified method name
                    size_t dotPos = methodToCall.find('.');
                    std::string methodNamePart = methodToCall.substr(dotPos + 1);
                    methodToCall = actualClassName + "." + methodNamePart;

                    pendingParams.clear();
                }

                // Add method call instruction
                bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::INVOKEVIRTUAL, methodToCall));

                // Store the result if needed
                if (!tacInst.result.empty()) {
                    bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
                }
            } else if (tacInst.op.empty()) {
                // Handle simple assignment
                if (typeTracker.directClassNames.find(tacInst.arg1) == typeTracker.directClassNames.end()) {
                    addLoadInstruction(tacInst.arg1);
                    bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
                }
            }
        }

        // Handle block exits
        if (block->trueExit) {
            bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::GOTO, block->trueExit->name));
        } else if (stop) {
            bytecodeBlock->addInstruction(std::make_unique<BCInstruction>(OpCode::STOP));
        }

        blocks.emplace_back(std::move(bytecodeBlock));
    }
}

// Helper function to convert opcode to string
std::string getOpcodeName(OpCode code) {
    switch (code) {
        case OpCode::ILOAD:
            return "iload";
        case OpCode::ICONST:
            return "iconst";
        case OpCode::ISTORE:
            return "istore";
        case OpCode::IADD:
            return "iadd";
        case OpCode::ISUB:
            return "isub";
        case OpCode::IMUL:
            return "imul";
        case OpCode::IDIV:
            return "idiv";
        case OpCode::ILT:
            return "ilt";
        case OpCode::IGT:
            return "igt";
        case OpCode::IEQ:
            return "ieq";
        case OpCode::IAND:
            return "iand";
        case OpCode::IOR:
            return "ior";
        case OpCode::INOT:
            return "inot";
        case OpCode::GOTO:
            return "goto";
        case OpCode::IFFALSEGOTO:
            return "iffalsegoto";
        case OpCode::INVOKEVIRTUAL:
            return "invokevirtual";
        case OpCode::IRETURN:
            return "ireturn";
        case OpCode::PRINT:
            return "print";
        case OpCode::STOP:
            return "stop";
        default:
            throw std::runtime_error("Unknown opcode" + std::to_string(static_cast<int>(code)));
    }
}

void BCInstruction::print(std::ofstream& outFile) const {
    outFile << getOpcodeName(id);
    if (!argument.empty()) {
        outFile << " " << argument;
    }
    outFile << std::endl;
}

void BCBlock::print(std::ofstream& outFile) const {
    outFile << name << ":" << std::endl;
    for (size_t i = 0; i < instructions.size(); i++) {
        outFile << i << ":  ";
        instructions[i]->print(outFile);
    }
    outFile << std::endl;
}

void BCProgram::print(std::ofstream& outFile) const {
    for (const auto& method : blocks) {
        method->print(outFile);
    }
}
