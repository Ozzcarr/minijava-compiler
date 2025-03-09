#include "BytecodeGenerator.h"
#include <iostream>

std::string getOpcodeName(OpCode code);


void BCProgram::generateBytecode(const ControlFlowGraph &cfg, const SymbolTable &symbolTable) {
    // Process each basic block in the CFG
    for (const auto& block : cfg.getBlocks()) {
        // Create a new method for each entry block
        auto method = std::make_unique<BCMethod>(block->name);
        bool stop = true;

        // Parse class and method names
        std::string className = block->name.substr(0, block->name.find('.'));
        std::string methodName = block->name.substr(block->name.find('.') + 1);

        // Handle method parameters
        if (symbolTable.hasClass(className)) {
            Class cls = symbolTable.getClass(className);
            if (cls.hasMethod(methodName)) {
                for (const auto& param : cls.getMethod(methodName).getParameters()) {
                    method->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, param.getName()));
                }
            }
        }

        // Helper function to add load instruction based on argument type
        auto addLoadInstruction = [&method](const std::string& arg) {
            OpCode opType = (arg.find_first_not_of("0123456789") == std::string::npos) ? 
                            OpCode::ICONST : OpCode::ILOAD;
            method->addInstruction(std::make_unique<BCInstruction>(opType, arg));
        };

        // Process TAC instructions in this block
        for (const auto& tacInst : block->getTacInstructions()) {
            if (tacInst.op == "print") {
                addLoadInstruction(tacInst.arg1);
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::PRINT));
            } 
            else if (tacInst.op == "return") {
                addLoadInstruction(tacInst.arg1);
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::IRETURN));
                stop = false;
            }
            else if (tacInst.op == " + " || tacInst.op == " - " || tacInst.op == " * " || tacInst.op == " < " ||
                     tacInst.op == " > " || tacInst.op == " && " || tacInst.op == " || " || tacInst.op == " == ") {
                // Determine operation type
                OpCode op;
                std::string arg1 = tacInst.arg1;
                std::string arg2 = tacInst.arg2;

                if (tacInst.op == " + ") op = OpCode::IADD;
                else if (tacInst.op == " - ") op = OpCode::ISUB;
                else if (tacInst.op == " * ") op = OpCode::IMUL;
                else if (tacInst.op == " < ") op = OpCode::ILT;
                else if (tacInst.op == " > ") op = OpCode::IGT;
                else if (tacInst.op == " == ") op = OpCode::IEQ;
                else if (tacInst.op == " && ") {
                    op = OpCode::IAND;
                    arg1 = (arg1 == "true") ? "1" : (arg1 == "false") ? "0" : arg1;
                    arg2 = (arg2 == "true") ? "1" : (arg2 == "false") ? "0" : arg2;
                } 
                else if (tacInst.op == " || ") {
                    op = OpCode::IOR;
                    arg1 = (arg1 == "true") ? "1" : (arg1 == "false") ? "0" : arg1;
                    arg2 = (arg2 == "true") ? "1" : (arg2 == "false") ? "0" : arg2;
                }

                // Load operands and perform operation
                addLoadInstruction(arg1);
                addLoadInstruction(arg2);
                method->addInstruction(std::make_unique<BCInstruction>(op));
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
            }
            else if (tacInst.op == "!") {
                std::string arg1 = (tacInst.arg1 == "true") ? "1" : 
                                  (tacInst.arg1 == "false") ? "0" : tacInst.arg1;
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::ICONST, arg1));
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::INOT));
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
            }
            else if (tacInst.op == "if") {
                addLoadInstruction(tacInst.arg1);
                method->addInstruction(std::make_unique<BCInstruction>(
                    OpCode::IFFALSEGOTO, block->falseExit->name));
            }
            else if (tacInst.op == "call") {
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::INVOKEVIRTUAL, tacInst.arg1));
                if (!tacInst.result.empty()) {
                    method->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
                }
            }
        }

        // Handle block exits (jumps)
        if (block->trueExit) {
            method->addInstruction(std::make_unique<BCInstruction>(OpCode::GOTO, block->trueExit->name));
        } else if (stop) {
            method->addInstruction(std::make_unique<BCInstruction>(OpCode::STOP));
        }

        // Add the method to the program
        methods.emplace_back(std::move(method));
    }

    // TODO: Second pass to resolve jump targets
}

void BCInstruction::print(std::ofstream &outFile) const {
    // Print opcode name instead of binary value
    outFile << getOpcodeName(id) << " ";

    // Print argument if it exists
    if (!argument.empty()) {
        outFile << argument;
    }
    outFile << std::endl;
}

void BCMethod::print(std::ofstream &outFile) const {
    // Print method header
    outFile << name << ":" <<  std::endl;

    // Print each instruction with line numbers
    for (size_t i = 0; i < instructions.size(); i++) {
        outFile << i << ": ";
        instructions[i]->print(outFile);
    }
    outFile << std::endl;
}

void BCProgram::print(std::ofstream &outFile) const {
    // Print each method
    for (const auto& method : methods) {
        method->print(outFile);
    }
}

// Helper function to convert opcode to string
std::string getOpcodeName(OpCode code) {
    switch(code) {
        case OpCode::ILOAD: return "iload";
        case OpCode::ICONST: return "iconst";
        case OpCode::ISTORE: return "istore";
        case OpCode::IADD: return "iadd";
        case OpCode::ISUB: return "isub";
        case OpCode::IMUL: return "imul";
        case OpCode::IDIV: return "idiv";
        case OpCode::ILT: return "ilt";
        case OpCode::IGT: return "igt";
        case OpCode::IEQ: return "ieq";
        case OpCode::IAND: return "iand";
        case OpCode::IOR: return "ior";
        case OpCode::INOT: return "inot";
        case OpCode::GOTO: return "goto";
        case OpCode::IFFALSEGOTO: return "iffalse goto";
        case OpCode::INVOKEVIRTUAL: return "invokevirtual";
        case OpCode::IRETURN: return "ireturn";
        case OpCode::PRINT: return "print";
        case OpCode::STOP: return "stop";
        default: return "unknown";
    }
}
