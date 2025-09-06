#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum class TokenType {
    // 字面量
    NUMBER, STRING, IDENTIFIER,
    
    // 关键字
    PRINT, INPUT, IF, ELSE, FOR, WHILE, DEF, RETURN, TRUE, FALSE, NONE,
    
    // 操作符
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    ASSIGN, EQUAL, NOT_EQUAL, LESS, GREATER,
    
    // 分隔符
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, DOT, COLON, SEMICOLON,
    
    // 其他
    NEWLINE, EOF_TOKEN, COMMENT
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;
    
    Token(TokenType t, const std::string& v, int l = 0, int c = 0)
        : type(t), value(v), line(l), col(c) {}
};

class Lexer {
private:
    std::string source;
    size_t pos;
    int line;
    int col;
    
    char currentChar() const;
    char peekChar() const;
    char peekChar2() const;
    void advance();
    Token readNumber();
    Token readString();
    Token readIdentifier();
    TokenType getKeywordType(const std::string& identifier);
    
public:
    Lexer(const std::string& sourceCode);
    std::vector<Token> tokenize();
};

#endif