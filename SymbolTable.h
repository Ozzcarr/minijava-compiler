#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

// Class representing a variable
class Variable {
   public:
    Variable(const std::string &name, const std::string &type) : name(name), type(type) {}

    std::string getName() const { return name; }
    std::string getType() const { return type; }

   private:
    std::string name;
    std::string type;
};

// Class representing a method
class Method {
   public:
    Method(const std::string &name, const std::string &returnType) : name(name), returnType(returnType) {}

    std::string getName() const { return name; }
    std::string getReturnType() const { return returnType; }
    void addParameter(const Variable &param) { parameters.push_back(param); }
    void addLocalVariable(const Variable &var, int lineNumber) { localVariables.push_back({var, lineNumber}); }
    const std::vector<Variable> &getParameters() const { return parameters; }
    const std::vector<std::pair<Variable, int>> &getLocalVariables() const { return localVariables; }

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
    Class(const std::string &name) : name(name) {}

    std::string getName() const { return name; }
    void addMethod(const Method &method) { methods.push_back(method); }
    void addVariable(const Variable &variable) { variables.push_back(variable); }
    const std::vector<Method> &getMethods() const { return methods; }
    const std::vector<Variable> &getVariables() const { return variables; }

   private:
    std::string name;
    std::vector<Method> methods;
    std::vector<Variable> variables;
};

// Symbol table class
class SymbolTable {
   public:
    void addClass(const Class &cls) { classes[cls.getName()] = cls; }

    std::string getVariableType(const std::string &identifier, const std::string &method,
                                const std::string &className) {
        auto classIt = classes.find(className);
        if (classIt != classes.end()) {
            // Class variables
            const std::vector<Variable> &variables = classIt->second.getVariables();
            auto varIt = std::find_if(variables.begin(), variables.end(),
                                      [&](const Variable &v) { return v.getName() == identifier; });
            if (varIt != variables.end()) {
                return varIt->getType();
            }

            // Method variables
            const std::vector<Method> &methods = classIt->second.getMethods();
            auto methodIt =
                std::find_if(methods.begin(), methods.end(), [&](const Method &m) { return m.getName() == method; });

            if (methodIt != methods.end()) {
                // Method parameters
                const std::vector<Variable> &parameters = methodIt->getParameters();
                auto paramIt = std::find_if(parameters.begin(), parameters.end(),
                                            [&](const Variable &v) { return v.getName() == identifier; });

                if (paramIt != parameters.end()) {
                    return paramIt->getType();
                } else {
                    // Local variables
                    const std::vector<std::pair<Variable, int>> &localVariables = methodIt->getLocalVariables();
                    auto localVarIt = std::find_if(
                        localVariables.begin(), localVariables.end(),
                        [&](const std::pair<Variable, int> &v) { return v.first.getName() == identifier; });
                    if (localVarIt != localVariables.end()) {
                        return localVarIt->first.getType();
                    }
                }
            }
        }

        // Check if identifier is a class
        classIt = classes.find(identifier);
        if (classIt != classes.end()) {
            return identifier;
        }

        // Variable not found
        throw std::runtime_error("Variable not found: " + identifier);
    }

    std::string getMethodReturnType(const std::string &className, const std::string &methodName) const {
        auto classIt = classes.find(className);
        if (classIt != classes.end()) {
            const std::vector<Method> &methods = classIt->second.getMethods();
            auto methodIt = std::find_if(methods.begin(), methods.end(),
                                         [&](const Method &m) { return m.getName() == methodName; });
            if (methodIt != methods.end()) {
                return methodIt->getReturnType();
            } else {
                throw std::runtime_error("Method not found: " + methodName);
            }
        } else {
            throw std::runtime_error("Class not found: " + className);
        }
    }

    const std::unordered_map<std::string, Class> &getClasses() const { return classes; }

   private:
    std::unordered_map<std::string, Class> classes;
};

#endif  // SYMBOL_TABLE_H