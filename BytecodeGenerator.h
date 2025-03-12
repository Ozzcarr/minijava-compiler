#ifndef BYTECODEGENERATOR_H
#define BYTECODEGENERATOR_H

#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "IntermediateRepresentation.h"
#include "SymbolTable.h"

enum class OpCode : uint8_t {
    ILOAD = 0,           // Load integer
    ICONST = 1,          // Load integer constant
    ISTORE = 2,          // Store integer
    IADD = 3,            // Integer addition
    ISUB = 4,            // Integer subtraction
    IMUL = 5,            // Integer multiplication
    IDIV = 6,            // Integer division
    ILT = 7,             // Integer less than
    IGT = 8,             // Integer greater than
    IEQ = 9,             // Integer equal
    IAND = 10,           // Integer AND
    IOR = 11,            // Integer OR
    INOT = 12,           // Integer NOT
    GOTO = 13,           // Unconditional jump
    IFFALSEGOTO = 14,    // Conditional jump
    INVOKEVIRTUAL = 15,  // Method call
    IRETURN = 16,        // Return integer
    PRINT = 17,          // Print integer or boolean
    STOP = 18            // End execution
};

class BCMethod;
class BCInstruction;

class BCProgram {
   private:
    // std::unordered_map<std::string, std::unique_ptr<BCMethod>> methods; // <class name, method> -> Class.Method
    std::vector<std::unique_ptr<BCMethod>> methods;

   public:
    /**
     * @brief Generates bytecode from the control flow graph.
     * @param cfg The control flow graph to convert.
     */
    void generateBytecode(const ControlFlowGraph &cfg, const SymbolTable &symbolTable);

    /**
     * @brief Prints the program to a file.
     * @param outFile The file to print the program to.
     */
    void print(std::ofstream &outFile) const;

    const std::vector<std::unique_ptr<BCMethod>>& getMethods() const { return methods; }
};

class BCMethod {
   private:
    std::vector<std::unique_ptr<BCInstruction>> instructions;
    std::string name;

   public:
    BCMethod(const std::string &name) : name(name) {}

    /**
     * @brief Adds an instruction to the method.
     * @param instruction The instruction to add.
     */
    inline void addInstruction(std::unique_ptr<BCInstruction> instruction) {
        instructions.push_back(std::move(instruction));
    }

    /**
     * @brief Prints the method to a file.
     * @param outFile The file to print the method to.
     */
    void print(std::ofstream &outFile) const;

    /**
     * @brief Gets the instructions of the method.
     * @return The instructions of the method.
     */
    const std::vector<std::unique_ptr<BCInstruction>> &getInstructions() const { return instructions; }

    const std::string& getName() const { return name; }
};

class BCInstruction {
   private:
    OpCode id;
    std::string argument;

   public:
    BCInstruction(OpCode id, const std::string &argument = "") : id(id), argument(argument) {}

    /**
     * @brief Prints the instruction to a file.
     * @param outFile The file to print the instruction to.
     */
    void print(std::ofstream &outFile) const;

    /**
     * @brief Gets the opcode of the instruction.
     * @return The opcode of the instruction.
     */
    OpCode getOpcode() const { return id; }

    /**
     * @brief Gets the argument of the instruction.
     * @return The argument of the instruction.
     */
    const std::string &getArgument() const { return argument; }
};

#endif  // BYTECODEGENERATOR_H
