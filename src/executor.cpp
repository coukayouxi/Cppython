#include "executor.h"
#include "parser.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cmath>

Executor::Executor(bool isInteractive) : interactiveMode(isInteractive) {
    fastIO();
}

Value Executor::evaluateExpression(const ExprNode* expr) {
    if (auto literal = dynamic_cast<const LiteralExpr*>(expr)) {
        return evaluateLiteral(literal);
    } else if (auto identifier = dynamic_cast<const IdentifierExpr*>(expr)) {
        return evaluateIdentifier(identifier);
    } else if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
        return evaluateBinary(binary);
    } else if (auto call = dynamic_cast<const CallExpr*>(expr)) {
        return evaluateCall(call);
    }
    
    return Value();
}

Value Executor::evaluateLiteral(const LiteralExpr* literal) {
    switch (literal->type) {
        case TokenType::NUMBER:
            return Value(std::stod(literal->value));
        case TokenType::STRING:
            return Value(literal->value);
        case TokenType::TRUE:
            return Value(true);
        case TokenType::FALSE:
            return Value(false);
        default:
            return Value(literal->value);
    }
}

Value Executor::evaluateIdentifier(const IdentifierExpr* identifier) {
    auto it = variables.find(identifier->name);
    if (it != variables.end()) {
        return it->second;
    }
    return Value(); // None
}

Value Executor::evaluateBinary(const BinaryExpr* binary) {
    Value left = evaluateExpression(binary->left.get());
    Value right = evaluateExpression(binary->right.get());
    
    switch (binary->op) {
        case TokenType::PLUS:
            if (left.type == Value::Type::STRING || right.type == Value::Type::STRING) {
                return Value(left.toString() + right.toString());
            } else {
                return Value(left.toNumber() + right.toNumber());
            }
        case TokenType::MINUS:
            return Value(left.toNumber() - right.toNumber());
        case TokenType::MULTIPLY:
            return Value(left.toNumber() * right.toNumber());
        case TokenType::DIVIDE:
            return Value(left.toNumber() / right.toNumber());
        case TokenType::MODULO:
            return Value(std::fmod(left.toNumber(), right.toNumber()));
        default:
            return Value();
    }
}

Value Executor::evaluateCall(const CallExpr* call) {
    std::string funcName = dynamic_cast<IdentifierExpr*>(call->callee.get())->name;
    
    if (funcName == "input") {
        // 内置input函数
        if (!call->arguments.empty()) {
            Value prompt = evaluateExpression(call->arguments[0].get());
            fastPutString(prompt.toString());
        }
        std::string input = fastGetString();
        return Value(input);
    }
    
    if (funcName == "print") {
        // print函数调用（虽然通常作为语句处理，但也支持作为表达式）
        std::string output;
        for (size_t i = 0; i < call->arguments.size(); i++) {
            Value arg = evaluateExpression(call->arguments[i].get());
            if (i > 0) output += " ";
            output += arg.toString();
        }
        fastPutString(output + "\n");
        return Value(); // print返回None
    }
    
    throw std::runtime_error("Function " + funcName + " is not defined");
}

void Executor::executeStatement(const StmtNode* stmt) {
    if (auto printStmt = dynamic_cast<const PrintStmt*>(stmt)) {
        executePrint(printStmt);
    } else if (auto assignStmt = dynamic_cast<const AssignStmt*>(stmt)) {
        executeAssignment(assignStmt);
    } else if (auto exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
        // 只在交互模式下输出表达式结果
        Value result = evaluateExpression(exprStmt->expression.get());
        if (interactiveMode && result.type != Value::Type::NONE) {
            fastPutString(result.toString() + "\n");
        }
    }
}

void Executor::executePrint(const PrintStmt* printStmt) {
    // 无空格分隔连接所有参数
    std::string output;
    for (size_t i = 0; i < printStmt->expressions.size(); i++) {
        Value value = evaluateExpression(printStmt->expressions[i].get());
        output += value.toString();
    }
    output += "\n";
    fastPutString(output);
}

void Executor::executeAssignment(const AssignStmt* assignStmt) {
    Value value = evaluateExpression(assignStmt->value.get());
    variables[assignStmt->variable] = value;
}

void Executor::execute(const std::vector<std::unique_ptr<StmtNode>>& statements) {
    for (const auto& stmt : statements) {
        executeStatement(stmt.get());
    }
}

std::string Executor::fastGetString() {
    std::string result;
    std::getline(std::cin, result);
    return result;
}

void Executor::fastPutString(const std::string& str) {
    std::cout << str;
    std::cout.flush();
}

// 快速IO优化
void Executor::fastIO() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

std::string Value::toString() const {
    switch (type) {
        case Type::NUMBER:
            {
                std::ostringstream oss;
                if (number == static_cast<long long>(number)) {
                    oss << static_cast<long long>(number);
                } else {
                    oss << number;
                }
                return oss.str();
            }
        case Type::STRING:
            return string_value;
        case Type::BOOLEAN:
            return boolean ? "True" : "False";
        case Type::NONE:
        default:
            return "None";
    }
}

double Value::toNumber() const {
    switch (type) {
        case Type::NUMBER:
            return number;
        case Type::STRING:
            try {
                return std::stod(string_value);
            } catch (...) {
                return 0.0;
            }
        case Type::BOOLEAN:
            return boolean ? 1.0 : 0.0;
        case Type::NONE:
        default:
            return 0.0;
    }
}

bool Value::toBoolean() const {
    switch (type) {
        case Type::NUMBER:
            return number != 0.0;
        case Type::STRING:
            return !string_value.empty();
        case Type::BOOLEAN:
            return boolean;
        case Type::NONE:
        default:
            return false;
    }
}