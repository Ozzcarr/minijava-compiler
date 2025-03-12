#include "StackMachineInterpreter.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

int main(int argc, char **argv) {
    StackMachineInterpreter interpreter;

    // Default to output.bc, but allow override via command line
    std::string bytecodeFile = "output.bc";
    if (argc > 1) {
        bytecodeFile = argv[1];
    }

    // Load bytecode from the specified file
    if (!interpreter.loadBytecode(bytecodeFile)) {
        return 1;
    }

    // Execute the loaded program
    int result = interpreter.execute();
    return result;
}

bool StackMachineInterpreter::loadBytecode(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open bytecode file: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::string currentMethod;
    std::vector<std::pair<OpCode, std::string>> instructions;

    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        // Check if this is a method declaration (ends with :)
        if (line.back() == ':') {
            // Save previous method if it exists
            if (!currentMethod.empty()) {
                methods[currentMethod] = instructions;
                instructions.clear();
            }

            // Set new current method
            currentMethod = line.substr(0, line.size() - 1);
            continue;
        }

        // Parse instruction line (format: "lineNum: opcode [argument]")
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string instructionPart = line.substr(colonPos + 1);

            // Trim leading whitespace
            size_t firstNonSpace = instructionPart.find_first_not_of(" \t");
            if (firstNonSpace != std::string::npos) {
                instructionPart = instructionPart.substr(firstNonSpace);
            }

            // Parse opcode and argument
            std::string opcodeName;
            std::string argument;

            std::istringstream iss(instructionPart);
            iss >> opcodeName;

            // Get the rest as argument (if any)
            std::getline(iss >> std::ws, argument);

            // Convert opcode name to OpCode enum
            OpCode opcode;
            if (opcodeName == "iload")
                opcode = OpCode::ILOAD;
            else if (opcodeName == "iconst")
                opcode = OpCode::ICONST;
            else if (opcodeName == "istore")
                opcode = OpCode::ISTORE;
            else if (opcodeName == "iadd")
                opcode = OpCode::IADD;
            else if (opcodeName == "isub")
                opcode = OpCode::ISUB;
            else if (opcodeName == "imul")
                opcode = OpCode::IMUL;
            else if (opcodeName == "idiv")
                opcode = OpCode::IDIV;
            else if (opcodeName == "ilt")
                opcode = OpCode::ILT;
            else if (opcodeName == "igt")
                opcode = OpCode::IGT;
            else if (opcodeName == "ieq")
                opcode = OpCode::IEQ;
            else if (opcodeName == "iand")
                opcode = OpCode::IAND;
            else if (opcodeName == "ior")
                opcode = OpCode::IOR;
            else if (opcodeName == "inot")
                opcode = OpCode::INOT;
            else if (opcodeName == "goto")
                opcode = OpCode::GOTO;
            else if (opcodeName == "iffalsegoto")
                opcode = OpCode::IFFALSEGOTO;
            else if (opcodeName == "invokevirtual")
                opcode = OpCode::INVOKEVIRTUAL;
            else if (opcodeName == "ireturn")
                opcode = OpCode::IRETURN;
            else if (opcodeName == "print")
                opcode = OpCode::PRINT;
            else if (opcodeName == "stop")
                opcode = OpCode::STOP;
            else {
                std::cerr << "Unknown opcode: " << opcodeName << std::endl;
                continue;
            }

            instructions.push_back({opcode, argument});
        }
    }

    // Save the last method
    if (!currentMethod.empty()) {
        methods[currentMethod] = instructions;
    }

    file.close();
    return true;
}

int StackMachineInterpreter::execute() {
    // Reset state
    reset();

    // Check if we have any methods
    if (methods.empty()) {
        std::cerr << "No methods found in bytecode" << std::endl;
        return -1;
    }

    currentMethod = methods.begin()->first;

    // Start execution from the main method
    programCounter = 0;
    running = true;

    // Execute instructions until program terminates
    while (running) {
        if (!executeInstruction()) {
            std::cerr << "Execution error at method: " << currentMethod << ", address: " << programCounter << std::endl;
            break;
        }
    }

    return 0;
}

bool StackMachineInterpreter::executeInstruction() {
    // Check if we're out of bounds
    if (programCounter >= methods[currentMethod].size()) {
        std::cerr << "Program counter out of bounds: " << programCounter << std::endl;
        running = false;
        return false;
    }

    // Get the current instruction
    const auto &instruction = methods[currentMethod][programCounter];
    OpCode opcode = instruction.first;
    const std::string &argument = instruction.second;

    // Execute the instruction
    switch (opcode) {
        case OpCode::ILOAD: {
            // Handle boolean literals
            if (argument == "true") {
                operandStack.push(StackValue(1, true));
            } else if (argument == "false") {
                operandStack.push(StackValue(0, true));
            } else if (localVariables.find(argument) == localVariables.end()) {
                std::cerr << "Variable not found: " << argument << std::endl;
                return false;
            } else {
                operandStack.push(localVariables[argument]);
            }
            programCounter++;
            break;
        }
        case OpCode::ICONST: {
            // Load constant onto stack - always an integer
            operandStack.push(StackValue(std::stoi(argument)));
            programCounter++;
            break;
        }
        case OpCode::ISTORE: {
            // Store top of stack in variable
            if (operandStack.empty()) {
                std::cerr << "Stack underflow on ISTORE" << std::endl;
                return false;
            }
            StackValue stackVal = operandStack.top();
            localVariables[argument] = stackVal;
            operandStack.pop();
            programCounter++;
            break;
        }
        case OpCode::IADD: {
            // Add top two values on stack
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on IADD" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue(a.value + b.value, false));
            programCounter++;
            break;
        }
        case OpCode::ISUB: {
            // Subtract top value from second value
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on ISUB" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue(a.value - b.value, false));
            programCounter++;
            break;
        }
        case OpCode::IMUL: {
            // Multiply top two values
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on IMUL" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue(a.value * b.value, false));
            programCounter++;
            break;
        }
        case OpCode::IDIV: {
            // Divide second value by top value
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on IDIV" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            if (b.value == 0) {
                std::cerr << "Division by zero" << std::endl;
                return false;
            }
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue(a.value / b.value, false));
            programCounter++;
            break;
        }
        case OpCode::ILT: {
            // Less than comparison
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on ILT" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue(a.value < b.value ? 1 : 0, true));
            programCounter++;
            break;
        }
        case OpCode::IGT: {
            // Greater than comparison
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on IGT" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue(a.value > b.value ? 1 : 0, true));
            programCounter++;
            break;
        }
        case OpCode::IEQ: {
            // Equality comparison
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on IEQ" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue(a.value == b.value ? 1 : 0, true));
            programCounter++;
            break;
        }
        case OpCode::IAND: {
            // Logical AND
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on IAND" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue((a.value != 0 && b.value != 0) ? 1 : 0, true));
            programCounter++;
            break;
        }
        case OpCode::IOR: {
            // Logical OR
            if (operandStack.size() < 2) {
                std::cerr << "Stack underflow on IOR" << std::endl;
                return false;
            }
            StackValue b = operandStack.top();
            operandStack.pop();
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue((a.value != 0 || b.value != 0) ? 1 : 0, true));
            programCounter++;
            break;
        }
        case OpCode::INOT: {
            // Logical NOT
            if (operandStack.empty()) {
                std::cerr << "Stack underflow on INOT" << std::endl;
                return false;
            }
            StackValue a = operandStack.top();
            operandStack.pop();

            operandStack.push(StackValue(a.value == 0 ? 1 : 0, true));
            programCounter++;
            break;
        }
        case OpCode::GOTO: {
            if (!jumpToBlock(argument)) {
                std::cerr << "Failed to jump to method: " << argument << std::endl;
                return false;
            }
            break;
        }
        case OpCode::IFFALSEGOTO: {
            // Conditional jump if top of stack is false (0)
            if (operandStack.empty()) {
                std::cerr << "Stack underflow on IFFALSEGOTO" << std::endl;
                return false;
            }

            int condition = operandStack.top().value;
            operandStack.pop();

            if (condition == 0) {
                if (!jumpToBlock(argument)) {
                    std::cerr << "Failed to jump to method: " << argument << std::endl;
                    return false;
                }
            } else {
                programCounter++;
            }
            break;
        }
        case OpCode::INVOKEVIRTUAL: {
            // Push current method and address to stack frame
            stackFrame.push({currentMethod, programCounter + 1, localVariables});
            if (!jumpToBlock(argument)) {
                std::cerr << "Failed to jump to method: " << argument << std::endl;
                return false;
            }
            break;
        }
        case OpCode::IRETURN: {
            if (operandStack.empty()) {
                std::cerr << "Stack underflow on IRETURN" << std::endl;
                return false;
            }

            if (stackFrame.empty()) {
                std::cerr << "Call stack underflow on IRETURN" << std::endl;
                running = false;
                return false;
            } else {
                // Restore state
                StackFrame frame = stackFrame.top();
                stackFrame.pop();

                currentMethod = frame.method;
                programCounter = frame.returnAddress;
                localVariables = frame.localVariables;
            }
            break;
        }
        case OpCode::PRINT: {
            // Print top of stack
            if (operandStack.empty()) {
                std::cerr << "Stack underflow on PRINT" << std::endl;
                return false;
            }

            const StackValue &val = operandStack.top();

            // If it's a boolean type, print true/false, otherwise print the number
            if (val.isBoolean) {
                std::cout << (val.value == 1 ? "true" : "false") << std::endl;
            } else {
                std::cout << val.value << std::endl;
            }

            operandStack.pop();
            programCounter++;
            break;
        }
        case OpCode::STOP: {
            // Stop execution
            running = false;
            return true;
        }
        default: {
            std::cerr << "Unknown opcode: " << static_cast<int>(opcode) << std::endl;
            return false;
        }
    }

    return true;
}

bool StackMachineInterpreter::jumpToBlock(const std::string &methodName) {
    // Check if block exists
    if (methods.find(methodName) == methods.end()) {
        std::cerr << "Block not found: " << methodName << std::endl;
        return false;
    }

    currentMethod = methodName;
    programCounter = 0;
    return true;
}

void StackMachineInterpreter::reset() {
    // Clear runtime state
    while (!operandStack.empty()) operandStack.pop();
    while (!stackFrame.empty()) stackFrame.pop();
    localVariables.clear();
    currentMethod = "";
    programCounter = 0;
    running = false;
}

StackValue StackMachineInterpreter::getVariable(const std::string &name) const {
    auto it = localVariables.find(name);
    if (it == localVariables.end()) {
        return StackValue(0);  // Default value for undefined variables
    }
    return it->second;
}
