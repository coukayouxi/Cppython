#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"
#include <unordered_map>
#include <string>
#include <vector>

class Value {
public:
    enum class Type {
        NUMBER,
        STRING,
        BOOLEAN,
        NONE
    };
    
    Type type;
    union {
        double number;
        bool boolean;
    };
    std::string string_value;
    
    Value() : type(Type::NONE) {}
    Value(double n) : type(Type::NUMBER), number(n) {}
    Value(const std::string& s) : type(Type::STRING), string_value(s) {}
    Value(bool b) : type(Type::BOOLEAN), boolean(b) {}
    
    std::string toString() const;
    double toNumber() const;
    bool toBoolean() const;
};

class Executor {
private:
    std::unordered_map<std::string, Value> variables;
    bool interactiveMode;
    
    Value evaluateExpression(const ExprNode* expr);
    Value evaluateLiteral(const LiteralExpr* literal);
    Value evaluateIdentifier(const IdentifierExpr* identifier);
    Value evaluateBinary(const BinaryExpr* binary);
    Value evaluateCall(const CallExpr* call);
    
    void executeStatement(const StmtNode* stmt);
    void executePrint(const PrintStmt* printStmt);
    void executeAssignment(const AssignStmt* assignStmt);
    
public:
    Executor(bool isInteractive = false);
    void setInteractiveMode(bool interactive) { interactiveMode = interactive; }
    void execute(const std::vector<std::unique_ptr<StmtNode>>& statements);
    
    // 快速输入输出
    static std::string fastGetString();
    static void fastPutString(const std::string& str);
    static void fastIO();
};

#endif