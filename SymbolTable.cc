#include "SymbolTable.h"

const Method &Class::getMethod(const std::string &methodName) const {
    auto it = std::find_if(methods.begin(), methods.end(), [&](const Method &m) { return m.getName() == methodName; });
    return *it;
}

const Class &SymbolTable::getClass(const std::string &className) const {
    auto it = classes.find(className);
    if (it != classes.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Class not found: " + className);
    }
}

const Class &SymbolTable::getOccurenceOfClass(const std::string &className, int occurence) const {
    int count = 0;
    for (const auto &cls : classes) {
        if (cls.first == className) {
            count++;
            if (count == occurence) {
                return cls.second;
            }
        }
    }
    throw std::runtime_error("Class occurence not found: " + className);
}

std::string SymbolTable::getVariableType(const std::string &identifier, const std::string &method,
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
                auto localVarIt =
                    std::find_if(localVariables.begin(), localVariables.end(),
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
    return "";
}

std::string SymbolTable::getMethodReturnType(const std::string &className, const std::string &methodName) const {
    auto classIt = classes.find(className);
    if (classIt != classes.end()) {
        const std::vector<Method> &methods = classIt->second.getMethods();
        auto methodIt =
            std::find_if(methods.begin(), methods.end(), [&](const Method &m) { return m.getName() == methodName; });
        if (methodIt != methods.end()) {
            return methodIt->getReturnType();
        } else {
            throw std::runtime_error("Method not found: " + methodName);
        }
    } else {
        throw std::runtime_error("Class not found: " + className);
    }
}
