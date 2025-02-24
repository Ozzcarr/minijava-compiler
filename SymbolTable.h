#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>



// Class representing a variable
class Variable {
   public:
    /**
     * @brief Constructs a Variable with a name and type.
     * @param name The name of the variable.
     * @param type The type of the variable.
     */
    Variable(const std::string &name, const std::string &type) : name(name), type(type) {}

    /**
     * @brief Gets the name of the variable.
     * @return The name of the variable.
     */
    std::string getName() const { return name; }

    /**
     * @brief Gets the type of the variable.
     * @return The type of the variable.
     */
    std::string getType() const { return type; }

   private:
    std::string name;
    std::string type;
};

// Class representing a method
class Method {
   public:
    /**
     * @brief Constructs a Method with a name and return type.
     * @param name The name of the method.
     * @param returnType The return type of the method.
     */
    Method(const std::string &name, const std::string &returnType) : name(name), returnType(returnType) {}

    /**
     * @brief Gets the name of the method.
     * @return The name of the method.
     */
    std::string getName() const { return name; }

    /**
     * @brief Gets the return type of the method.
     * @return The return type of the method.
     */
    std::string getReturnType() const { return returnType; }

    /**
     * @brief Adds a parameter to the method.
     * @param param The parameter to add.
     */
    void addParameter(const Variable &param) { parameters.push_back(param); }

    /**
     * @brief Adds a local variable to the method.
     * @param var The local variable to add.
     * @param lineNumber The line number where the variable is declared.
     */
    void addLocalVariable(const Variable &var, int lineNumber) { localVariables.push_back({var, lineNumber}); }

    /**
     * @brief Gets the parameters of the method.
     * @return A reference to the vector of parameters.
     */
    const std::vector<Variable> &getParameters() const { return parameters; }

    /**
     * @brief Gets the local variables of the method.
     * @return A reference to the vector of local variables.
     */
    const std::vector<std::pair<Variable, int>> &getLocalVariables() const { return localVariables; }

    /**
     * @brief Checks if a variable is declared before a specific line number.
     * @param varName The name of the variable.
     * @param lineNumber The line number to check against.
     * @return True if the variable is declared before the line number, otherwise false.
     */
    bool isVariableDeclaredBefore(const std::string &varName, int lineNumber) const {
        return std::any_of(localVariables.begin(), localVariables.end(), [&](const std::pair<Variable, int> &v) {
            return v.first.getName() == varName && v.second < lineNumber;
        });
    }

    /**
     * @brief Checks if a variable is a local variable.
     * @param varName The name of the variable.
     * @return True if the variable is a local variable, otherwise false.
     */
    bool isLocalVariable(const std::string &varName) const {
        return std::any_of(localVariables.begin(), localVariables.end(),
                           [&](const std::pair<Variable, int> &v) { return v.first.getName() == varName; });
    }

   private:
    std::string name;
    std::string returnType;
    std::vector<Variable> parameters;
    std::vector<std::pair<Variable, int>> localVariables;
};

// Class representing a class
class Class {
   public:
    Class() = default;  // Default constructor

    /**
     * @brief Constructs a Class with a name.
     * @param name The name of the class.
     */
    Class(const std::string &name) : name(name) {}

    /**
     * @brief Gets the name of the class.
     * @return The name of the class.
     */
    std::string getName() const { return name; }

    /**
     * @brief Adds a method to the class.
     * @param method The method to add.
     */
    void addMethod(const Method &method) { methods.push_back(method); }

    /**
     * @brief Adds a variable to the class.
     * @param variable The variable to add.
     */
    void addVariable(const Variable &variable) { variables.push_back(variable); }

    /**
     * @brief Gets the methods of the class.
     * @return A reference to the vector of methods.
     */
    const std::vector<Method> &getMethods() const { return methods; }

    /**
     * @brief Gets the variables of the class.
     * @return A reference to the vector of variables.
     */
    const std::vector<Variable> &getVariables() const { return variables; }

    /**
     * @brief Checks if the class has a method with the specified name.
     * @param methodName The name of the method.
     * @return True if the class has the method, otherwise false.
     */
    bool hasMethod(const std::string &methodName) const {
        return std::any_of(methods.begin(), methods.end(), [&](const Method &m) { return m.getName() == methodName; });
    }

    /**
     * @brief Gets a method by name.
     * @param methodName The name of the method.
     * @return The method with the specified name.
     * @throws std::runtime_error if the method is not found.
     */
    const Method &getMethod(const std::string &methodName) const;

   private:
    std::string name;
    std::vector<Method> methods;
    std::vector<Variable> variables;
};

// Symbol table class
class SymbolTable {
   public:
    /** */
     * @brief Adds a class to the symbol table.
     * @param cls The class to add.
     */
    void addClass(const Class &cls) { classes.emplace(cls.getName(), cls); }

    /**
     * @brief Checks if a class exists in the symbol table.
     * @param className The name of the class.
     * @return True if the class exists, otherwise false.
     */
    bool hasClass(const std::string &className) const { return classes.find(className) != classes.end(); }

    /**
     * @brief Gets a class by name.
     * @param className The name of the class.
     * @return The class with the specified name.
     */
    const Class &getClass(const std::string &className);

    /**
     * @brief Gets the type of a variable.
     * @param identifier The name of the variable.
     * @param method The name of the method containing the variable.
     * @param className The name of the class containing the method.
     * @return The type of the variable.
     * @throws std::runtime_error if the variable is not found.
     */
    std::string getVariableType(const std::string &identifier, const std::string &method, const std::string &className);

    /**
     * @brief Gets the return type of a method.
     * @param className The name of the class containing the method.
     * @param methodName The name of the method.
     * @return The return type of the method.
     * @throws std::runtime_error if the method or class is not found.
     */
    std::string getMethodReturnType(const std::string &className, const std::string &methodName) const;

    /**
     * @brief Gets all classes in the symbol table.
     * @return A reference to the unordered multimap of classes.
     */
    const std::unordered_multimap<std::string, Class> &getClasses() const { return classes; }

   private:
    std::unordered_multimap<std::string, Class> classes;
};

#endif  // SYMBOL_TABLE_H