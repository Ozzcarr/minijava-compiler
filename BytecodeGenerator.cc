#include "BytecodeGenerator.h"

#include <iostream>
#include <unordered_map>
#include <unordered_set>

std::string getOpcodeName(OpCode code);

void BCProgram::generateBytecode(const ControlFlowGraph& cfg, const SymbolTable& symbolTable) {
    // Map to track temporary variables and their types
    std::unordered_map<std::string, std::string> tempVarTypes;
    // Map to track whether a variable is a direct class name or a temporary class reference
    std::unordered_map<std::string, bool> isClassReference;
    // Set of direct class names in the symbol table
    std::unordered_set<std::string> directClassNames;

    // Initialize direct class names from symbol table
    for (const auto& cls : symbolTable.getAllClasses()) {
        directClassNames.insert(cls.getName());
    }

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
        auto addLoadInstruction = [&](const std::string& arg) {
            // std::cout << "Adding load instruction for: '" << arg << "'" << std::endl;
            // Only skip direct class names, not variables that might contain class references
            if (directClassNames.find(arg) != directClassNames.end()) {
                // std::cout << "  Skipping direct class name load" << std::endl;
                return;
            }

            // Remove class reference check here - we need to load reference values for operations
            OpCode opType = (arg.find_first_not_of("0123456789") == std::string::npos) ? OpCode::ICONST : OpCode::ILOAD;
            // std::cout << "  Using opcode: " << (opType == OpCode::ICONST ? "ICONST" : "ILOAD") << std::endl;
            method->addInstruction(std::make_unique<BCInstruction>(opType, arg));
        }; 


        // Process TAC instructions in this block
        std::vector<std::string> pendingParams;
        std::string targetMethod;

        // Flag to identify if this is the main method
        bool isMainMethod = methodName == "main";

        // First pass - identify direct class names versus temporary variables
        for (const auto& tacInst : block->getTacInstructions()) {
            // Check for class instantiations - they appear as special instructions in the TAC
            // where the operation is empty and arg1 is a class name from the symbol table
            if (tacInst.op.empty() && directClassNames.find(tacInst.arg1) != directClassNames.end()) {
                // Direct assignment of a class name
                tempVarTypes[tacInst.result] = tacInst.arg1;
                isClassReference[tacInst.result] = true;
            }
            // Also handle class references passed through assignments
            else if (tacInst.op.empty() && isClassReference.find(tacInst.arg1) != isClassReference.end() &&
                     isClassReference[tacInst.arg1]) {
                tempVarTypes[tacInst.result] = tempVarTypes[tacInst.arg1];
                isClassReference[tacInst.result] = true;
            }
            // Special handling for "new" operations in params (especially in the main method)
            else if (tacInst.op == "param" && tacInst.arg1.substr(0, 2) == "_t") {
                // Check if this is a parameter that will be used in a method call
                // but we don't know its type yet - store it for second pass
            }
            // Track new object expressions directly from method calls in main
            else if (tacInst.op == "call" && isMainMethod) {
                // We need to scan backward to find the last "param" instruction
                // and check if it's a direct class instantiation
                for (auto it = pendingParams.begin(); it != pendingParams.end(); ++it) {
                    const std::string& param = *it;
                    if (param.substr(0, 2) == "_t") {
                        // Try to find this temporary in the block's TAC instructions
                        for (const auto& prevInst : block->getTacInstructions()) {
                            if (prevInst.result == param && prevInst.op == "new") {
                                // This is a new object expression, record its type
                                tempVarTypes[param] = prevInst.arg1;
                                isClassReference[param] = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Second pass - generate bytecode
        pendingParams.clear(); // Reset pending params for the second pass

        for (const auto& tacInst : block->getTacInstructions()) {
            if (tacInst.op == "param") {
                // Store parameter for upcoming method call
                pendingParams.push_back(tacInst.arg1);
            } else if (tacInst.op == "print") {
                addLoadInstruction(tacInst.arg1);
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::PRINT));
            } else if (tacInst.op == "return") {
                addLoadInstruction(tacInst.arg1);
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::IRETURN));
                stop = false;
            } else if (tacInst.op == " + " || tacInst.op == " - " || tacInst.op == " * " || tacInst.op == " < " ||
                       tacInst.op == " > " || tacInst.op == " && " || tacInst.op == " || " || tacInst.op == " == ") {
                // Determine operation type
                OpCode op;
                std::string arg1 = tacInst.arg1;
                std::string arg2 = tacInst.arg2;

                if (tacInst.op == " + ")
                    op = OpCode::IADD;
                else if (tacInst.op == " - ")
                    op = OpCode::ISUB;
                else if (tacInst.op == " * ")
                    op = OpCode::IMUL;
                else if (tacInst.op == " < ")
                    op = OpCode::ILT;
                else if (tacInst.op == " > ")
                    op = OpCode::IGT;
                else if (tacInst.op == " == ")
                    op = OpCode::IEQ;
                else if (tacInst.op == " && ") {
                    op = OpCode::IAND;
                    arg1 = (arg1 == "true") ? "1" : (arg1 == "false") ? "0" : arg1;
                    arg2 = (arg2 == "true") ? "1" : (arg2 == "false") ? "0" : arg2;
                } else if (tacInst.op == " || ") {
                    op = OpCode::IOR;
                    arg1 = (arg1 == "true") ? "1" : (arg1 == "false") ? "0" : arg1;
                    arg2 = (arg2 == "true") ? "1" : (arg2 == "false") ? "0" : arg2;
                }

                // Load operands and perform operation
                addLoadInstruction(arg1);
                addLoadInstruction(arg2);
                method->addInstruction(std::make_unique<BCInstruction>(op));
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
            } else if (tacInst.op == "!") {
                std::string arg1 = (tacInst.arg1 == "true") ? "1" : (tacInst.arg1 == "false") ? "0" : tacInst.arg1;
                addLoadInstruction(arg1);
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::INOT));
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
            } else if (tacInst.op == "if") {
                addLoadInstruction(tacInst.arg1);
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::IFFALSEGOTO, block->falseExit->name));
            } else if (tacInst.op == "call") {
                // Process the method call
                std::string methodToCall = tacInst.arg1;

                if (!pendingParams.empty()) {
                    // Get first parameter (the class instance)
                    std::string classRef = pendingParams[0];
                    std::string actualClassName;

                    // Enhanced class name resolution:
                    // 1. Check if we already know this is a class reference
                    if (tempVarTypes.find(classRef) != tempVarTypes.end()) {
                        actualClassName = tempVarTypes[classRef];
                    }
                    // 2. Check if it's a direct class name
                    else if (directClassNames.find(classRef) != directClassNames.end()) {
                        actualClassName = classRef;
                    }
                    // 3. Special case for main method: look for the class in the method name directly
                    else if (isMainMethod && methodToCall.find('.') != std::string::npos) {
                        size_t dotPos = methodToCall.find('.');
                        actualClassName = methodToCall.substr(0, dotPos);
                    }
                    // 4. Final fallback - use the parameter as is
                    else {
                        actualClassName = classRef;
                    }

                    // Add remaining parameters (excluding the first one which is the class)
                    for (int i = pendingParams.size() - 1; i > 0; i--) {
                        addLoadInstruction(pendingParams[i]);
                    }

                    // Form the fully qualified method name using the actual class name
                    if (methodToCall.find('.') == std::string::npos) {
                        methodToCall = actualClassName + "." + methodToCall;
                    } else {
                        // If the method call already has a class name, replace it with the actual class name
                        size_t dotPos = methodToCall.find('.');
                        std::string methodNamePart = methodToCall.substr(dotPos + 1);
                        methodToCall = actualClassName + "." + methodNamePart;
                    }

                    pendingParams.clear();
                }

                // Add method call instruction
                method->addInstruction(std::make_unique<BCInstruction>(OpCode::INVOKEVIRTUAL, methodToCall));

                // Store the result if needed
                if (!tacInst.result.empty()) {
                    method->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
                }
            } else if (tacInst.op == "new") {
                // Handle new object creation - just record the type
                tempVarTypes[tacInst.result] = tacInst.arg1;
                isClassReference[tacInst.result] = true;
                // No bytecode - this is just for type tracking
            } else if (tacInst.op.empty()) {
                // Only skip loading for direct class names, not variables containing references
                if (directClassNames.find(tacInst.arg1) == directClassNames.end()) {
                    // std::cout << "Assigning " << tacInst.arg1 << " to " << tacInst.result << std::endl;
                    addLoadInstruction(tacInst.arg1);
                    method->addInstruction(std::make_unique<BCInstruction>(OpCode::ISTORE, tacInst.result));
                }
                // Always clear class reference flag for arithmetic operands
                isClassReference[tacInst.result] = false;

                // Preserve type information for method calls
                if (tempVarTypes.find(tacInst.arg1) != tempVarTypes.end()) {
                    tempVarTypes[tacInst.result] = tempVarTypes[tacInst.arg1];
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
}

void BCInstruction::print(std::ofstream& outFile) const {
    // Print opcode name instead of binary value
    outFile << getOpcodeName(id);

    // Print argument if it exists
    if (!argument.empty()) {
        outFile << " " << argument;
    }
    outFile << std::endl;
}

void BCMethod::print(std::ofstream& outFile) const {
    // Print method header
    outFile << name << ":" << std::endl;

    // Print each instruction with line numbers
    for (size_t i = 0; i < instructions.size(); i++) {
        outFile << i << ":  ";
        instructions[i]->print(outFile);
    }
    outFile << std::endl;
}

void BCProgram::print(std::ofstream& outFile) const {
    // Print each method
    for (const auto& method : methods) {
        method->print(outFile);
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
