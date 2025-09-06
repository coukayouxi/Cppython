#include "executor.h"
#include "parser.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <regex>

Executor::Executor(bool isInteractive) : interactiveMode(isInteractive) {
    fastIO();
}

Value Executor::evaluateExpression(const ExprNode* expr) {
    if (auto literal = dynamic_cast<const LiteralExpr*>(expr)) {
        return evaluateLiteral(literal);
    } else if (auto fstring = dynamic_cast<const FStringExpr*>(expr)) {
        return evaluateFString(fstring);
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

Value Executor::parseAndEvaluateSimpleExpression(const std::string& expr_str) {
    // 去除空白字符
    std::string expr = expr_str;
    expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());
    
    if (expr.empty()) {
        return Value("None");
    }
    
    // 简单的数字
    if (std::all_of(expr.begin(), expr.end(), [](char c) { 
        return std::isdigit(c) || c == '.'; 
    })) {
        try {
            return Value(std::stod(expr));
        } catch (...) {
            return Value(expr);
        }
    }
    
    // 简单的变量
    if (std::all_of(expr.begin(), expr.end(), [](char c) { 
        return std::isalnum(c) || c == '_'; 
    })) {
        auto it = variables.find(expr);
        if (it != variables.end()) {
            return it->second;
        }
        return Value("{" + expr + "}");
    }
    
    // 处理二元操作表达式
    // 查找操作符（从右到左，因为加减优先级低于乘除）
    for (int i = expr.length() - 1; i >= 0; i--) {
        char op = expr[i];
        if (op == '+' || op == '-') {
            std::string left_str = expr.substr(0, i);
            std::string right_str = expr.substr(i + 1);
            
            if (!left_str.empty() && !right_str.empty()) {
                try {
                    Value left_val = parseAndEvaluateSimpleExpression(left_str);
                    Value right_val = parseAndEvaluateSimpleExpression(right_str);
                    
                    if (op == '+') {
                        if (left_val.type == Value::Type::STRING || right_val.type == Value::Type::STRING) {
                            return Value(left_val.toString() + right_val.toString());
                        } else {
                            return Value(left_val.toNumber() + right_val.toNumber());
                        }
                    } else if (op == '-') {
                        return Value(left_val.toNumber() - right_val.toNumber());
                    }
                } catch (...) {
                    continue;
                }
            }
        }
    }
    
    // 查找乘除操作符
    for (int i = expr.length() - 1; i >= 0; i--) {
        char op = expr[i];
        if (op == '*' || op == '/' || op == '%') {
            std::string left_str = expr.substr(0, i);
            std::string right_str = expr.substr(i + 1);
            
            if (!left_str.empty() && !right_str.empty()) {
                try {
                    Value left_val = parseAndEvaluateSimpleExpression(left_str);
                    Value right_val = parseAndEvaluateSimpleExpression(right_str);
                    
                    if (op == '*') {
                        return Value(left_val.toNumber() * right_val.toNumber());
                    } else if (op == '/') {
                        return Value(left_val.toNumber() / right_val.toNumber());
                    } else if (op == '%') {
                        return Value(std::fmod(left_val.toNumber(), right_val.toNumber()));
                    }
                } catch (...) {
                    continue;
                }
            }
        }
    }
    
    // 如果无法解析，返回原始表达式
    return Value("{" + expr_str + "}");
}

Value Executor::evaluateFString(const FStringExpr* fstring) {
    std::string template_str = fstring->template_string;
    std::string result = "";
    
    size_t pos = 0;
    while (pos < template_str.length()) {
        if (template_str[pos] == '\\') {
            // 处理转义字符
            if (pos + 1 < template_str.length()) {
                pos++;
                switch (template_str[pos]) {
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'v': result += '\v'; break;
                    case '\\': result += '\\'; break;
                    case '"': result += '"'; break;
                    case '\'': result += '\''; break;
                    case '{': result += '{'; break; // 转义大括号
                    case '}': result += '}'; break; // 转义大括号
                    default: result += template_str[pos]; break;
                }
            } else {
                result += template_str[pos];
            }
            pos++;
        } else if (template_str[pos] == '{') {
            // 找到表达式开始
            size_t expr_start = pos + 1;
            int brace_count = 1;
            size_t expr_end = expr_start;
            
            // 找到匹配的右括号
            while (expr_end < template_str.length() && brace_count > 0) {
                if (template_str[expr_end] == '{') {
                    brace_count++;
                } else if (template_str[expr_end] == '}') {
                    brace_count--;
                }
                if (brace_count > 0) expr_end++;
            }
            
            if (brace_count == 0) {
                // 提取表达式
                std::string expr_str = template_str.substr(expr_start, expr_end - expr_start);
                
                // 解析并评估表达式
                Value expr_value = parseAndEvaluateSimpleExpression(expr_str);
                result += expr_value.toString();
                
                pos = expr_end + 1;
            } else {
                result += template_str[pos];
                pos++;
            }
        } else {
            result += template_str[pos];
            pos++;
        }
    }
    
    return Value(result);
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

Value Executor::evaluateEval(const std::vector<std::unique_ptr<ExprNode>>& arguments) {
    if (arguments.empty()) {
        throw std::runtime_error("eval() missing required argument");
    }
    
    // 获取要评估的表达式字符串
    Value expr_value = evaluateExpression(arguments[0].get());
    std::string expr_str = expr_value.toString();
    
    // 如果表达式是纯数字，直接返回
    if (std::all_of(expr_str.begin(), expr_str.end(), [](char c) { 
        return std::isdigit(c) || c == '.'; 
    })) {
        try {
            return Value(std::stod(expr_str));
        } catch (...) {
            // 继续下面的解析
        }
    }
    
    try {
        // 创建新的词法分析器和解析器来处理表达式
        Lexer lexer(expr_str);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        // 只解析表达式（不是完整语句）
        auto expr_node = parser.parseExpressionPublic();
        
        // 评估表达式
        return evaluateExpression(expr_node.get());
    } catch (const std::exception& e) {
        // 如果解析失败，尝试简单表达式解析
        return parseAndEvaluateSimpleExpression(expr_str);
    }
}

Value Executor::evaluateExec(const std::vector<std::unique_ptr<ExprNode>>& arguments) {
    if (arguments.empty()) {
        throw std::runtime_error("exec() missing required argument");
    }
    
    // 获取要执行的代码字符串
    Value code_value = evaluateExpression(arguments[0].get());
    std::string code_str = code_value.toString();
    
    try {
        // 确保代码以换行符结尾
        if (!code_str.empty() && code_str.back() != '\n') {
            code_str += '\n';
        }
        
        // 创建新的词法分析器和解析器来处理代码
        Lexer lexer(code_str);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto statements = parser.parse();
        
        // 执行语句（在当前执行器上下文中）
        for (const auto& stmt : statements) {
            executeStatement(stmt.get());
        }
        
        return Value(); // exec返回None
    } catch (const std::exception& e) {
        throw std::runtime_error("exec error: " + std::string(e.what()));
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
        // print函数调用
        std::string output;
        for (size_t i = 0; i < call->arguments.size(); i++) {
            Value arg = evaluateExpression(call->arguments[i].get());
            if (i > 0) output += " ";
            output += arg.toString();
        }
        fastPutString(output + "\n");
        return Value(); // print返回None
    }
    
    if (funcName == "eval") {
        return evaluateEval(call->arguments);
    }
    
    if (funcName == "exec") {
        return evaluateExec(call->arguments);
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