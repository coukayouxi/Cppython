#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>
#include <sstream>

// 值类型
class Value {
public:
    enum class Type {
        NUMBER,
        STRING,
        BOOLEAN,
        NONE,
        FILE_OBJECT,
        LIST  // 添加列表类型
    };
    
    Type type;
    union {
        double number;
        bool boolean;
    };
    std::string string_value;
    
    // 添加列表支持
    std::vector<Value> list_value;
    
    // 文件对象支持
    struct FileObject {
        std::string filename;
        std::string mode;
        bool is_binary;
        bool is_open;
        
        FileObject(const std::string& fname, const std::string& m, bool binary)
            : filename(fname), mode(m), is_binary(binary), is_open(true) {
        }
    };
    
    std::unique_ptr<FileObject> file_object;
    
    // 默认构造函数
    Value() : type(Type::NONE) {}
    
    // 数字构造函数
    Value(double n) : type(Type::NUMBER), number(n) {}
    
    // 字符串构造函数
    Value(const std::string& s) : type(Type::STRING), string_value(s) {}
    
    // 布尔构造函数
    Value(bool b) : type(Type::BOOLEAN), boolean(b) {}
    
    // 列表构造函数
    Value(const std::vector<Value>& list) : type(Type::LIST), list_value(list) {}
    
    // 文件对象构造函数
    Value(std::unique_ptr<FileObject> file_obj) : type(Type::FILE_OBJECT), file_object(std::move(file_obj)) {}
    
    // 拷贝构造函数
    Value(const Value& other) : type(other.type) {
        switch (type) {
            case Type::NUMBER:
                number = other.number;
                break;
            case Type::STRING:
                string_value = other.string_value;
                break;
            case Type::BOOLEAN:
                boolean = other.boolean;
                break;
            case Type::LIST:
                list_value = other.list_value;
                break;
            case Type::FILE_OBJECT:
                // 文件对象拷贝：只拷贝元数据
                if (other.file_object) {
                    file_object = std::make_unique<FileObject>(
                        other.file_object->filename,
                        other.file_object->mode,
                        other.file_object->is_binary
                    );
                    file_object->is_open = other.file_object->is_open;
                }
                break;
            case Type::NONE:
            default:
                break;
        }
    }
    
    // 拷贝赋值操作符
    Value& operator=(const Value& other) {
        if (this != &other) {
            type = other.type;
            switch (type) {
                case Type::NUMBER:
                    number = other.number;
                    break;
                case Type::STRING:
                    string_value = other.string_value;
                    break;
                case Type::BOOLEAN:
                    boolean = other.boolean;
                    break;
                case Type::LIST:
                    list_value = other.list_value;
                    break;
                case Type::FILE_OBJECT:
                    // 文件对象拷贝：只拷贝元数据
                    if (other.file_object) {
                        file_object = std::make_unique<FileObject>(
                            other.file_object->filename,
                            other.file_object->mode,
                            other.file_object->is_binary
                        );
                        file_object->is_open = other.file_object->is_open;
                    }
                    break;
                case Type::NONE:
                default:
                    break;
            }
        }
        return *this;
    }
    
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
    Value evaluateList(const ListExpr* list);
    Value evaluateIndex(const IndexExpr* index);
    Value evaluateFString(const FStringExpr* fstring);
    Value evaluateIdentifier(const IdentifierExpr* identifier);
    Value evaluateBinary(const BinaryExpr* binary);
    Value evaluateCall(const CallExpr* call);
    
    // 新增：eval和exec功能
    Value evaluateEval(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    Value evaluateExec(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    
    // 新增：with语句功能
    void executeWith(const WithStmt* withStmt);
    
    // 新增：文件操作功能
    Value evaluateOpen(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    Value evaluateFileRead(const std::string& filename, bool is_binary);
    Value evaluateFileWrite(const std::string& filename, const std::string& data, bool is_binary, const std::string& mode);
    
    // 新增：内置函数
    Value evaluateStr(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    Value evaluateRepr(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    Value evaluateInt(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    Value evaluateFloat(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    Value evaluateBool(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    Value evaluateLen(const std::vector<std::unique_ptr<ExprNode>>& arguments);
    
    // f-string表达式解析辅助函数
    Value parseAndEvaluateSimpleExpression(const std::string& expr_str);
    
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