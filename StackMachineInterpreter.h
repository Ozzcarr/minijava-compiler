#ifndef STACKMACHINEINTERPRETER_H
#define STACKMACHINEINTERPRETER_H

#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "BytecodeGenerator.h"

struct StackValue {
    int value;
    bool isBoolean;

    StackValue() : value(0), isBoolean(false) {}
    StackValue(int v, bool b = false) : value(v), isBoolean(b) {}
};

class StackMachineInterpreter {
   private:
    // Program structure
    std::unordered_map<std::string, std::vector<std::pair<OpCode, std::string>>> methods;

    // Runtime state
    std::stack<StackValue> operandStack;
    std::unordered_map<std::string, StackValue> localVariables;
    std::string currentMethod;
    size_t programCounter;
    bool running;

    // Method call stack for return addresses
    std::stack<std::pair<std::string, size_t>> callStack;

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
     * @param className Optional name of the class containing the main methodt
     * @return The return value of the program
     */
    int execute(const std::string &className = "");

    /**
     * @brief Executes a single instruction
     * @return True if execution should continue
     */
    bool executeInstruction();

    /**
     * @brief Jumps to a method
     * @param methodName The name of the method to jump to
     * @param returnAddress The address to return to after method execution
     */
    void jumpToMethod(const std::string &methodName, size_t returnAddress);

    /**
     * @brief Returns from a method call
     */
    void returnFromMethod();

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

    /**
     * @brief Dumps the current state of the interpreter for debugging
     */
    void dumpState() const;
};

#endif  // STACKMACHINEINTERPRETER_H
