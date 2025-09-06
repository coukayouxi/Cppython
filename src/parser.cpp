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
    
    if (match(TokenType::IDENTIFIER)) {
        auto expr = std::make_unique<IdentifierExpr>(previous().value);
        
        // 检查是否是函数调用
        if (match(TokenType::LPAREN)) {
            return parseCall(std::move(expr));
        }
        
        return expr;
    }
    
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw std::runtime_error("Expected expression at line " + std::to_string(peek().line));
}

std::unique_ptr<StmtNode> Parser::parseStatement() {
    if (peek().type == TokenType::PRINT) {
        return parsePrintStatement();
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