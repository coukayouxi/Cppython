#include "lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& sourceCode) 
    : source(sourceCode), pos(0), line(1), col(0) {
}

char Lexer::currentChar() const {
    if (pos >= source.length()) return '\0';
    return source[pos];
}

char Lexer::peekChar() const {
    if (pos + 1 >= source.length()) return '\0';
    return source[pos + 1];
}

char Lexer::peekChar2() const {
    if (pos + 2 >= source.length()) return '\0';
    return source[pos + 2];
}

void Lexer::advance() {
    if (pos < source.length()) {
        if (source[pos] == '\n') {
            line++;
            col = 0;
        } else {
            col++;
        }
        pos++;
    }
}

Token Lexer::readString() {
    char quote = currentChar();
    advance();
    
    // 检查三引号
    if (quote == '"' && pos + 1 < source.length() && 
        currentChar() == '"' && peekChar() == '"') {
        // 三引号字符串
        advance(); advance(); // 跳过第二个和第三个引号
        size_t start = pos;
        
        // 寻找结束的三引号
        while (pos + 2 < source.length()) {
            if (source[pos] == '"' && source[pos + 1] == '"' && source[pos + 2] == '"') {
                break;
            }
            advance();
        }
        
        std::string value = source.substr(start, pos - start);
        if (pos + 2 < source.length()) {
            advance(); advance(); advance(); // 跳过结束的三引号
        }
        
        return Token(TokenType::STRING, value, line, col);
    } else if (quote == '\'' && pos + 1 < source.length() && 
               currentChar() == '\'' && peekChar() == '\'') {
        // 三引号字符串（单引号）
        advance(); advance(); // 跳过第二个和第三个引号
        size_t start = pos;
        
        // 寻找结束的三引号
        while (pos + 2 < source.length()) {
            if (source[pos] == '\'' && source[pos + 1] == '\'' && source[pos + 2] == '\'') {
                break;
            }
            advance();
        }
        
        std::string value = source.substr(start, pos - start);
        if (pos + 2 < source.length()) {
            advance(); advance(); advance(); // 跳过结束的三引号
        }
        
        return Token(TokenType::STRING, value, line, col);
    } else {
        // 普通字符串
        size_t start = pos;
        
        while (currentChar() != quote && currentChar() != '\0' && currentChar() != '\n') {
            if (currentChar() == '\\' && peekChar() != '\0') {
                advance(); // 跳过转义字符
            }
            advance();
        }
        
        std::string value = source.substr(start, pos - start);
        if (currentChar() == quote) {
            advance(); // 跳过结束引号
        }
        
        return Token(TokenType::STRING, value, line, col);
    }
}

Token Lexer::readNumber() {
    size_t start = pos;
    bool hasDot = false;
    
    while (std::isdigit(currentChar()) || currentChar() == '.') {
        if (currentChar() == '.') {
            if (hasDot) break;
            hasDot = true;
        }
        advance();
    }
    
    return Token(TokenType::NUMBER, source.substr(start, pos - start), line, col);
}

Token Lexer::readIdentifier() {
    size_t start = pos;
    while (std::isalnum(currentChar()) || currentChar() == '_') {
        advance();
    }
    
    std::string identifier = source.substr(start, pos - start);
    TokenType type = getKeywordType(identifier);
    
    return Token(type, identifier, line, col);
}

TokenType Lexer::getKeywordType(const std::string& identifier) {
    if (identifier == "print") return TokenType::PRINT;
    if (identifier == "input") return TokenType::INPUT;
    if (identifier == "if") return TokenType::IF;
    if (identifier == "else") return TokenType::ELSE;
    if (identifier == "for") return TokenType::FOR;
    if (identifier == "while") return TokenType::WHILE;
    if (identifier == "def") return TokenType::DEF;
    if (identifier == "return") return TokenType::RETURN;
    if (identifier == "True") return TokenType::TRUE;
    if (identifier == "False") return TokenType::FALSE;
    if (identifier == "None") return TokenType::NONE;
    
    return TokenType::IDENTIFIER;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (pos < source.length()) {
        // 跳过空白字符（除了换行符）
        while (std::isspace(currentChar()) && currentChar() != '\n') {
            advance();
        }
        
        if (currentChar() == '\0') break;
        
        if (currentChar() == '#') {
            // 跳过注释
            while (currentChar() != '\n' && currentChar() != '\0') {
                advance();
            }
            continue;
        }
        
        if (std::isdigit(currentChar())) {
            tokens.push_back(readNumber());
            continue;
        }
        
        if (currentChar() == '"' || currentChar() == '\'') {
            tokens.push_back(readString());
            continue;
        }
        
        if (std::isalpha(currentChar()) || currentChar() == '_') {
            tokens.push_back(readIdentifier());
            continue;
        }
        
        // 操作符和分隔符
        switch (currentChar()) {
            case '+': tokens.emplace_back(TokenType::PLUS, "+", line, col); advance(); break;
            case '-': tokens.emplace_back(TokenType::MINUS, "-", line, col); advance(); break;
            case '*': tokens.emplace_back(TokenType::MULTIPLY, "*", line, col); advance(); break;
            case '/': tokens.emplace_back(TokenType::DIVIDE, "/", line, col); advance(); break;
            case '%': tokens.emplace_back(TokenType::MODULO, "%", line, col); advance(); break;
            case '=': 
                if (peekChar() == '=') {
                    advance(); advance();
                    tokens.emplace_back(TokenType::EQUAL, "==", line, col);
                } else {
                    tokens.emplace_back(TokenType::ASSIGN, "=", line, col);
                    advance();
                }
                break;
            case '!':
                if (peekChar() == '=') {
                    advance(); advance();
                    tokens.emplace_back(TokenType::NOT_EQUAL, "!=", line, col);
                } else {
                    advance();
                }
                break;
            case '<':
                tokens.emplace_back(TokenType::LESS, "<", line, col);
                advance();
                break;
            case '>':
                tokens.emplace_back(TokenType::GREATER, ">", line, col);
                advance();
                break;
            case '(': tokens.emplace_back(TokenType::LPAREN, "(", line, col); advance(); break;
            case ')': tokens.emplace_back(TokenType::RPAREN, ")", line, col); advance(); break;
            case '{': tokens.emplace_back(TokenType::LBRACE, "{", line, col); advance(); break;
            case '}': tokens.emplace_back(TokenType::RBRACE, "}", line, col); advance(); break;
            case '[': tokens.emplace_back(TokenType::LBRACKET, "[", line, col); advance(); break;
            case ']': tokens.emplace_back(TokenType::RBRACKET, "]", line, col); advance(); break;
            case ',': tokens.emplace_back(TokenType::COMMA, ",", line, col); advance(); break;
            case '.': tokens.emplace_back(TokenType::DOT, ".", line, col); advance(); break;
            case ':': tokens.emplace_back(TokenType::COLON, ":", line, col); advance(); break;
            case ';': tokens.emplace_back(TokenType::SEMICOLON, ";", line, col); advance(); break;
            case '\n': tokens.emplace_back(TokenType::NEWLINE, "\n", line, col); advance(); break;
            default: advance(); break;
        }
    }
    
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line, col);
    return tokens;
}