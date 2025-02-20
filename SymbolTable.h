#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

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
    const std::vector<Variable> &getParameters() const { return parameters; }

   private:
    std::string name;
    std::string returnType;
    std::vector<Variable> parameters;
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

    std::string getVariableType(const std::string &identifier) const {

        for (const auto &clsPair : classes) {
            const Class &cls = clsPair.second;
            const auto &variables = cls.getVariables();
            auto varIt = std::find_if(variables.begin(), variables.end(), [&](const Variable &var) {
                return var.getName() == identifier;
            });

            if (varIt != variables.end()) {
                return varIt->getType();
            }
        }
    throw std::runtime_error("Variable not found for identifier: " + identifier);
}
    std::string getMethodReturnType(const std::string &className, const std::string &methodName) const {
        auto classIt = classes.find(className);
        if (classIt != classes.end()) {
            const std::vector<Method> &methods = classIt->second.getMethods();
            auto methodIt = std::find_if(methods.begin(), methods.end(), [&](const Method &m) {
                return m.getName() == methodName;
            });
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