#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <vector>

// AST节点基类
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string toString() const = 0;
};

// 表达式节点
class ExprNode : public ASTNode {
public:
    virtual ~ExprNode() = default;
};

// 语句节点
class StmtNode : public ASTNode {
public:
    virtual ~StmtNode() = default;
};

// 字面量表达式
class LiteralExpr : public ExprNode {
public:
    std::string value;
    TokenType type;
    
    LiteralExpr(const std::string& val, TokenType t) : value(val), type(t) {}
    std::string toString() const override;
};

// f-string表达式
class FStringExpr : public ExprNode {
public:
    std::string template_string;
    
    FStringExpr(const std::string& tmpl) : template_string(tmpl) {}
    std::string toString() const override;
};

// 标识符表达式
class IdentifierExpr : public ExprNode {
public:
    std::string name;
    
    IdentifierExpr(const std::string& n) : name(n) {}
    std::string toString() const override;
};

// 二元操作表达式
class BinaryExpr : public ExprNode {
public:
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
    TokenType op;
    
    BinaryExpr(std::unique_ptr<ExprNode> l, TokenType o, std::unique_ptr<ExprNode> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    std::string toString() const override;
};

// 函数调用表达式
class CallExpr : public ExprNode {
public:
    std::unique_ptr<ExprNode> callee;
    std::vector<std::unique_ptr<ExprNode>> arguments;
    
    CallExpr(std::unique_ptr<ExprNode> c, std::vector<std::unique_ptr<ExprNode>> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
    std::string toString() const override;
};

// 打印语句
class PrintStmt : public StmtNode {
public:
    std::vector<std::unique_ptr<ExprNode>> expressions;
    
    PrintStmt(std::vector<std::unique_ptr<ExprNode>> exprs)
        : expressions(std::move(exprs)) {}
    std::string toString() const override;
};

// 赋值语句
class AssignStmt : public StmtNode {
public:
    std::string variable;
    std::unique_ptr<ExprNode> value;
    
    AssignStmt(const std::string& var, std::unique_ptr<ExprNode> val)
        : variable(var), value(std::move(val)) {}
    std::string toString() const override;
};

// 表达式语句
class ExprStmt : public StmtNode {
public:
    std::unique_ptr<ExprNode> expression;
    
    ExprStmt(std::unique_ptr<ExprNode> expr)
        : expression(std::move(expr)) {}
    std::string toString() const override;
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token advance();
    
    // 解析表达式
    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseComparison();
    std::unique_ptr<ExprNode> parseTerm();
    std::unique_ptr<ExprNode> parseFactor();
    std::unique_ptr<ExprNode> parseUnary();
    std::unique_ptr<ExprNode> parsePrimary();
    std::unique_ptr<ExprNode> parseCall(std::unique_ptr<ExprNode> callee);
    
    // 解析语句
    std::unique_ptr<StmtNode> parseStatement();
    std::unique_ptr<StmtNode> parsePrintStatement();
    std::unique_ptr<StmtNode> parseAssignmentStatement();
    
public:
    Parser(const std::vector<Token>& tokenList);
    std::vector<std::unique_ptr<StmtNode>> parse();
};

#endif