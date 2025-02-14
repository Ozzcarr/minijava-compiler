#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

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
    const std::unordered_map<std::string, Class> &getClasses() const { return classes; }

   private:
    std::unordered_map<std::string, Class> classes;
};

#endif  // SYMBOL_TABLE_H