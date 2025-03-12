#ifndef STACKMACHINEINTERPRETER_H
#define STACKMACHINEINTERPRETER_H

#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include "BytecodeGenerator.h"

struct StackValue {
    int value;
    bool isBoolean;

    StackValue() : value(0), isBoolean(false) {}
    StackValue(int v, bool b = false) : value(v), isBoolean(b) {}
};

struct StackFrame {
    std::string method;
    size_t returnAddress;
    std::unordered_map<std::string, StackValue> localVariables;
};

class StackMachineInterpreter {
   private:
    // Program structure
    std::map<std::string, std::vector<std::pair<OpCode, std::string>>> methods;

    // Runtime state
    std::stack<StackValue> operandStack;
    std::stack<StackFrame> stackFrame;
    std::unordered_map<std::string, StackValue> localVariables;
    std::string currentMethod;
    size_t programCounter;
    bool running;

   public:
    StackMachineInterpreter() : programCounter(0), running(false) {}

    /**
     * @brief Loads bytecode from a file
     * @param filename The file containing the bytecode
     * @return True if loading was successful
     */
    bool loadBytecode(const std::string &filename);

    /**
     * @brief Executes the loaded program starting from the main method
     * @return The return value of the program
     */
    int execute();

    /**
     * @brief Executes a single instruction
     * @return True if execution should continue
     */
    bool executeInstruction();

    /**
     * @brief Jumps to a block
     * @param methodName The name of the method to jump to
     * @return True if the jump was successful
     */
    bool jumpToBlock(const std::string &methodName);

    /**
     * @brief Resets the interpreter state
     */
    void reset();

    /**
     * @brief Gets the value of a local variable
     * @param name The name of the variable
     * @return The value of the variable
     */
    StackValue getVariable(const std::string &name) const;
};

#endif  // STACKMACHINEINTERPRETER_H
