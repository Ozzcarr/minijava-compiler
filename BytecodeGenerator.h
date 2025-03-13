#ifndef BYTECODEGENERATOR_H
#define BYTECODEGENERATOR_H

#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

class BCBlock;
class BCInstruction;

class BCProgram {
   private:
    std::vector<std::unique_ptr<BCBlock>> blocks;

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

    const std::vector<std::unique_ptr<BCBlock>> &getBlocks() const { return blocks; }
};

class BCBlock {
   private:
    std::vector<std::unique_ptr<BCInstruction>> instructions;
    std::string name;

   public:
    BCBlock(const std::string &name) : name(name) {}

    /**
     * @brief Adds an instruction to the block.
     * @param instruction The instruction to add.
     */
    inline void addInstruction(std::unique_ptr<BCInstruction> instruction) {
        instructions.push_back(std::move(instruction));
    }

    /**
     * @brief Prints the block to a file.
     * @param outFile The file to print the block to.
     */
    void print(std::ofstream &outFile) const;

    /**
     * @brief Gets the instructions of the block.
     * @return The instructions of the block.
     */
    const std::vector<std::unique_ptr<BCInstruction>> &getInstructions() const { return instructions; }

    const std::string &getName() const { return name; }
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

// Helper class to track variable types and class references
class TypeTracker {
   public:
    std::unordered_map<std::string, std::string> tempVarTypes;
    std::unordered_map<std::string, bool> isClassReference;
    std::unordered_set<std::string> directClassNames;

    TypeTracker(const SymbolTable &symbolTable) {
        for (const auto &cls : symbolTable.getAllClasses()) {
            directClassNames.insert(cls.getName());
        }
    }

    std::string resolveClassName(const std::string &ref) {
        if (tempVarTypes.find(ref) != tempVarTypes.end()) {
            return tempVarTypes[ref];
        }
        if (directClassNames.find(ref) != directClassNames.end()) {
            return ref;
        }
        return ref;
    }

    void trackAssignment(const std::string &result, const std::string &source) {
        if (directClassNames.find(source) != directClassNames.end()) {
            // Direct assignment of a class name
            tempVarTypes[result] = source;
            isClassReference[result] = true;
        } else if (isClassReference.find(source) != isClassReference.end() && isClassReference[source]) {
            tempVarTypes[result] = tempVarTypes[source];
            isClassReference[result] = true;
        }
    }

    void trackNewObject(const std::string &var, const std::string &className) {
        tempVarTypes[var] = className;
        isClassReference[var] = true;
    }
};

#endif  // BYTECODEGENERATOR_H
