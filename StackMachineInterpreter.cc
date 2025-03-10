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
    std::cout << "Program terminated with result: " << result << std::endl;

    return 0;
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

int StackMachineInterpreter::execute(const std::string &className) {
    // Reset state
    reset();

    // Construct the main method name based on the provided class name or find one
    std::string mainClass = className;
    std::string entryPoint;

    if (mainClass.empty()) {
        // Try to find a class with a main method
        for (const auto &[methodName, _] : methods) {
            // Look for methods that end with ".main"
            size_t dotPos = methodName.find('.');
            if (dotPos != std::string::npos && methodName.substr(dotPos + 1) == "main") {
                mainClass = methodName.substr(0, dotPos);
                entryPoint = methodName;
                break;
            }
        }
    } else {
        // Construct entry point from the provided class name
        entryPoint = mainClass + ".main";
    }

    // Check if we found a valid entry point
    if (entryPoint.empty() || methods.find(entryPoint) == methods.end()) {
        std::cerr << "Main method not found" << std::endl;
        return -1;
    }

    std::cout << "Starting execution from: " << entryPoint << std::endl;

    // Start execution from the main method
    currentMethod = entryPoint;
    programCounter = 0;
    running = true;

    // Execute instructions until program terminates
    while (running) {
        if (!executeInstruction()) {
            break;
        }
    }

    // Return the top value from the stack (if any)
    if (!operandStack.empty()) {
        return operandStack.top().value;
    }

    return 0;
}

bool StackMachineInterpreter::executeInstruction() {
    // Check if we're at the end of the method
    if (programCounter >= methods[currentMethod].size()) {
        if (callStack.empty()) {
            // End of program
            running = false;
            return false;
        } else {
            // Return to caller
            returnFromMethod();
            return true;
        }
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
                dumpState();
                std::cerr << "Variable not found: " << argument << std::endl;
                operandStack.push(StackValue(0));  // Default to 0 for undefined variables
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
            // Find the target method
            auto it = methods.find(argument);
            if (it == methods.end()) {
                // Try to find a label within the current method
                bool found = false;
                for (size_t i = 0; i < methods[currentMethod].size(); i++) {
                    // This is a simplification - in a real implementation, you'd need proper label handling
                    if (i == std::stoul(argument)) {
                        programCounter = i;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    std::cerr << "Jump target not found: " << argument << std::endl;
                    return false;
                }
            } else {
                // Jump to another method
                jumpToMethod(argument, programCounter + 1);
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
                // Find the target method
                auto it = methods.find(argument);
                if (it == methods.end()) {
                    // Try to find a label within the current method
                    bool found = false;
                    for (size_t i = 0; i < methods[currentMethod].size(); i++) {
                        // This is a simplification - in a real implementation, you'd need proper label handling
                        if (i == std::stoul(argument)) {
                            programCounter = i;
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        std::cerr << "Jump target not found: " << argument << std::endl;
                        return false;
                    }
                } else {
                    // Jump to another method
                    jumpToMethod(argument, programCounter + 1);
                }
            } else {
                programCounter++;
            }
            break;
        }
        case OpCode::INVOKEVIRTUAL: {
            // Call a method
            // Determine if the method call includes parameter information
            size_t paramPos = argument.find('(');
            int numParams = 0;
            std::string methodName = argument;

            if (paramPos != std::string::npos) {
                // Extract number of parameters
                size_t endParamPos = argument.find(')', paramPos);
                if (endParamPos != std::string::npos) {
                    std::string paramCount = argument.substr(paramPos + 1, endParamPos - paramPos - 1);
                    try {
                        numParams = std::stoi(paramCount);
                        // Trim the method name to remove parameter info
                        methodName = argument.substr(0, paramPos);
                    } catch (const std::invalid_argument &) {
                        std::cerr << "Invalid parameter count in method call: " << argument << std::endl;
                    }
                }
            }

            jumpToMethod(methodName, programCounter + 1);
            break;
        }
        case OpCode::IRETURN: {
            // Return from method with a value
            if (operandStack.empty()) {
                std::cerr << "Stack underflow on IRETURN" << std::endl;
                return false;
            }

            StackValue returnValue = operandStack.top();
            operandStack.pop();

            if (callStack.empty()) {
                // End of program
                operandStack.push(returnValue);
                running = false;
                return false;
            } else {
                // Return to caller
                returnFromMethod();
                operandStack.push(returnValue);
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
            return false;
        }
        default: {
            std::cerr << "Unknown opcode: " << static_cast<int>(opcode) << std::endl;
            return false;
        }
    }

    return true;
}

void StackMachineInterpreter::jumpToMethod(const std::string &methodName, size_t returnAddress) {
    // Check if method exists
    if (methods.find(methodName) == methods.end()) {
        // Try to find an alternative method
        std::string alternativeMethod;

        // Extract the method part (after the dot)
        size_t dotPos = methodName.find('.');
        if (dotPos != std::string::npos) {
            std::string methodPart = methodName.substr(dotPos + 1);

            // Look for any method ending with the same method name
            for (const auto &[name, _] : methods) {
                size_t nameDotPos = name.find('.');
                if (nameDotPos != std::string::npos && name.substr(nameDotPos + 1) == methodPart) {
                    alternativeMethod = name;
                    break;
                }
            }
        }

        if (!alternativeMethod.empty()) {
            std::cout << "(Should look up class name for method) Method not found: " << methodName
                      << ", using alternative: " << alternativeMethod << std::endl;

            // Save current state
            callStack.push({currentMethod, returnAddress});

            // Jump to alternative method
            currentMethod = alternativeMethod;
            programCounter = 0;
            return;
        }

        // No alternative found
        std::cerr << "Method not found: " << methodName << std::endl;
        running = false;
        return;
    }

    // Method exists, proceed normally
    callStack.push({currentMethod, returnAddress});
    currentMethod = methodName;
    programCounter = 0;
}

void StackMachineInterpreter::returnFromMethod() {
    if (callStack.empty()) {
        std::cerr << "Call stack underflow" << std::endl;
        running = false;
        return;
    }

    // Restore state
    auto [method, address] = callStack.top();
    callStack.pop();

    currentMethod = method;
    programCounter = address;
}

void StackMachineInterpreter::reset() {
    // Clear runtime state
    while (!operandStack.empty()) operandStack.pop();
    while (!callStack.empty()) callStack.pop();
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

void StackMachineInterpreter::dumpState() const {
    std::cout << "=== Interpreter State ===" << std::endl;
    std::cout << "Current Method: " << currentMethod << std::endl;
    std::cout << "Program Counter: " << programCounter << std::endl;
    std::cout << "Running: " << (running ? "true" : "false") << std::endl;

    std::cout << "Stack (top to bottom):" << std::endl;
    std::stack<StackValue> tempStack = operandStack;
    while (!tempStack.empty()) {
        const StackValue &val = tempStack.top();
        if (val.isBoolean) {
            std::cout << "  " << (val.value == 1 ? "true" : "false") << std::endl;
        } else {
            std::cout << "  " << val.value << std::endl;
        }
        tempStack.pop();
    }

    std::cout << "Local Variables:" << std::endl;
    for (const auto &[name, value] : localVariables) {
        if (value.isBoolean) {
            std::cout << "  " << name << " = " << (value.value == 1 ? "true" : "false") << std::endl;
        } else {
            std::cout << "  " << name << " = " << value.value << std::endl;
        }
    }

    std::cout << "Call Stack (top to bottom):" << std::endl;
    std::stack<std::pair<std::string, size_t>> tempCallStack = callStack;
    while (!tempCallStack.empty()) {
        const auto &[method, address] = tempCallStack.top();
        std::cout << "  " << method << " at " << address << std::endl;
        tempCallStack.pop();
    }

    std::cout << "=========================" << std::endl;
}
