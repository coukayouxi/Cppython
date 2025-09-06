#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokenList)
    : tokens(tokenList), current(0) {}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (peek().type == type) {
        Token result = peek();
        advance();
        return result;
    }
    throw std::runtime_error(message + " at line " + std::to_string(peek().line));
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

std::unique_ptr<ExprNode> Parser::parseExpression() {
    return parseComparison();
}

std::unique_ptr<ExprNode> Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL) ||
           match(TokenType::LESS) || match(TokenType::GREATER)) {
        TokenType op = previous().type;
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExprNode> Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        TokenType op = previous().type;
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExprNode> Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) || match(TokenType::MODULO)) {
        TokenType op = previous().type;
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExprNode> Parser::parseUnary() {
    return parsePrimary();
}

std::unique_ptr<ExprNode> Parser::parseCall(std::unique_ptr<ExprNode> callee) {
    std::vector<std::unique_ptr<ExprNode>> arguments;
    
    if (!match(TokenType::RPAREN)) {
        do {
            arguments.push_back(parseExpression());
        } while (match(TokenType::COMMA));
        
        consume(TokenType::RPAREN, "Expected ')' after arguments");
    }
    
    return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
}

// 添加列表解析
std::unique_ptr<ExprNode> Parser::parseList() {
    consume(TokenType::LBRACKET, "Expected '[' for list");
    
    std::vector<std::unique_ptr<ExprNode>> elements;
    
    if (!match(TokenType::RBRACKET)) {
        do {
            elements.push_back(parseExpression());
        } while (match(TokenType::COMMA));
        
        consume(TokenType::RBRACKET, "Expected ']' after list elements");
    }
    
    return std::make_unique<ListExpr>(std::move(elements));
}

// 添加索引解析
std::unique_ptr<ExprNode> Parser::parseIndex(std::unique_ptr<ExprNode> array) {
    consume(TokenType::LBRACKET, "Expected '[' for index");
    auto index = parseExpression();
    consume(TokenType::RBRACKET, "Expected ']' after index");
    
    return std::make_unique<IndexExpr>(std::move(array), std::move(index));
}

std::unique_ptr<ExprNode> Parser::parsePrimary() {
    if (match(TokenType::NUMBER)) {
        return std::make_unique<LiteralExpr>(previous().value, TokenType::NUMBER);
    }
    
    if (match(TokenType::STRING)) {
        return std::make_unique<LiteralExpr>(previous().value, TokenType::STRING);
    }
    
    if (match(TokenType::F_STRING)) {
        return std::make_unique<FStringExpr>(previous().value);
    }
    
    if (match(TokenType::TRUE)) {
        return std::make_unique<LiteralExpr>("True", TokenType::TRUE);
    }
    
    if (match(TokenType::FALSE)) {
        return std::make_unique<LiteralExpr>("False", TokenType::FALSE);
    }
    
    // 添加列表字面量支持
    if (match(TokenType::LBRACKET)) {
        current--; // 回退一个token
        return parseList();
    }
    
    if (match(TokenType::IDENTIFIER)) {
        // 修复：使用正确的类型转换
        auto expr = std::make_unique<IdentifierExpr>(previous().value);
        std::unique_ptr<ExprNode> base_expr = std::move(expr); // 转换为基类指针
        
        // 检查是否是函数调用或索引
        while (true) {
            if (match(TokenType::LPAREN)) {
                return parseCall(std::move(base_expr));
            } else if (match(TokenType::LBRACKET)) {
                current--; // 回退，让parseIndex处理
                base_expr = parseIndex(std::move(base_expr));
            } else {
                break;
            }
        }
        
        return base_expr;
    }
    
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw std::runtime_error("Expected expression at line " + std::to_string(peek().line));
}

// 添加with语句解析
std::unique_ptr<StmtNode> Parser::parseWithStatement() {
    consume(TokenType::WITH, "Expected 'with'");
    
    // 解析上下文表达式
    auto context_expr = parseExpression();
    
    // 可选的as子句
    std::string optional_vars = "";
    if (match(TokenType::AS)) {
        Token var_token = consume(TokenType::IDENTIFIER, "Expected identifier after 'as'");
        optional_vars = var_token.value;
    }
    
    consume(TokenType::COLON, "Expected ':' after with statement");
    consume(TokenType::NEWLINE, "Expected newline after with statement");
    
    // 解析with语句体（简化处理）
    std::vector<std::unique_ptr<StmtNode>> body;
    while (!isAtEnd() && peek().type != TokenType::NEWLINE && peek().type != TokenType::EOF_TOKEN) {
        body.push_back(parseStatement());
        if (peek().type == TokenType::NEWLINE) {
            advance();
        }
    }
    
    return std::make_unique<WithStmt>(std::move(context_expr), optional_vars, std::move(body));
}

std::unique_ptr<StmtNode> Parser::parseStatement() {
    if (peek().type == TokenType::PRINT) {
        return parsePrintStatement();
    }
    
    if (peek().type == TokenType::WITH) {  // 添加with语句处理
        return parseWithStatement();
    }
    
    if (peek().type == TokenType::IDENTIFIER && 
        current + 1 < tokens.size() && 
        tokens[current + 1].type == TokenType::ASSIGN) {
        return parseAssignmentStatement();
    }
    
    // 表达式语句
    auto expr = parseExpression();
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<StmtNode> Parser::parsePrintStatement() {
    consume(TokenType::PRINT, "Expected 'print'");
    consume(TokenType::LPAREN, "Expected '(' after 'print'");
    
    std::vector<std::unique_ptr<ExprNode>> expressions;
    
    if (peek().type != TokenType::RPAREN) {
        expressions.push_back(parseExpression());
        
        while (match(TokenType::COMMA)) {
            expressions.push_back(parseExpression());
        }
    }
    
    consume(TokenType::RPAREN, "Expected ')' after print arguments");
    
    return std::make_unique<PrintStmt>(std::move(expressions));
}

std::unique_ptr<StmtNode> Parser::parseAssignmentStatement() {
    Token identifier = consume(TokenType::IDENTIFIER, "Expected identifier");
    consume(TokenType::ASSIGN, "Expected '=' after identifier");
    
    auto value = parseExpression();
    
    return std::make_unique<AssignStmt>(identifier.value, std::move(value));
}

std::vector<std::unique_ptr<StmtNode>> Parser::parse() {
    std::vector<std::unique_ptr<StmtNode>> statements;
    
    while (!isAtEnd()) {
        if (peek().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        
        if (peek().type == TokenType::EOF_TOKEN) {
            break;
        }
        
        try {
            statements.push_back(parseStatement());
        } catch (const std::exception& e) {
            // 跳过到下一个语句
            while (!isAtEnd() && peek().type != TokenType::NEWLINE && peek().type != TokenType::EOF_TOKEN) {
                advance();
            }
            if (peek().type == TokenType::NEWLINE) {
                advance();
            }
            // 继续解析下一个语句而不是抛出异常
            continue;
        }
        
        if (peek().type == TokenType::NEWLINE) {
            advance();
        }
    }
    
    return statements;
}

std::string LiteralExpr::toString() const {
    return value;
}

std::string FStringExpr::toString() const {
    return "f\"" + template_string + "\"";
}

std::string IdentifierExpr::toString() const {
    return name;
}

// 添加ListExpr的toString实现
std::string ListExpr::toString() const {
    std::string result = "[";
    for (size_t i = 0; i < elements.size(); i++) {
        if (i > 0) result += ", ";
        result += elements[i]->toString();
    }
    result += "]";
    return result;
}

// 添加IndexExpr的toString实现
std::string IndexExpr::toString() const {
    return array->toString() + "[" + index->toString() + "]";
}

std::string BinaryExpr::toString() const {
    std::string opStr;
    switch (op) {
        case TokenType::PLUS: opStr = "+"; break;
        case TokenType::MINUS: opStr = "-"; break;
        case TokenType::MULTIPLY: opStr = "*"; break;
        case TokenType::DIVIDE: opStr = "/"; break;
        case TokenType::MODULO: opStr = "%"; break;
        case TokenType::EQUAL: opStr = "=="; break;
        case TokenType::NOT_EQUAL: opStr = "!="; break;
        case TokenType::LESS: opStr = "<"; break;
        case TokenType::GREATER: opStr = ">"; break;
        default: opStr = "?"; break;
    }
    return "(" + left->toString() + " " + opStr + " " + right->toString() + ")";
}

std::string CallExpr::toString() const {
    std::string result = callee->toString() + "(";
    for (size_t i = 0; i < arguments.size(); i++) {
        if (i > 0) result += ", ";
        result += arguments[i]->toString();
    }
    result += ")";
    return result;
}

std::string PrintStmt::toString() const {
    std::string result = "print(";
    for (size_t i = 0; i < expressions.size(); i++) {
        if (i > 0) result += ", ";
        result += expressions[i]->toString();
    }
    result += ")";
    return result;
}

std::string AssignStmt::toString() const {
    return variable + " = " + value->toString();
}

std::string ExprStmt::toString() const {
    return expression->toString();
}

std::string WithStmt::toString() const {
    std::string result = "with " + context_expr->toString();
    if (!optional_vars.empty()) {
        result += " as " + optional_vars;
    }
    result += ":";
    return result;
}