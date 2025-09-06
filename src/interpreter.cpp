#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "executor.h"
#include "utils.h"
#include <iostream>
#include <fstream>

PythonInterpreter::PythonInterpreter() {
    executor = std::make_unique<Executor>(false);  // 文件执行模式
}

PythonInterpreter::~PythonInterpreter() = default;

bool PythonInterpreter::executeFile(const std::string& filename) {
    try {
        std::string source = Utils::readFile(filename);
        if (source.empty()) {
            return false;
        }
        
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto statements = parser.parse();
        
        // 确保是文件执行模式（不输出表达式结果）
        executor->setInteractiveMode(false);
        executor->execute(statements);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

void PythonInterpreter::interactiveMode() {
    std::cout << "CPPython 1.0.3 (simplified interpreter)" << std::endl;
    std::cout << "Type \"help\", \"copyright\", \"credits\" or \"license\" for more information." << std::endl;
    
    // 设置为交互模式
    executor->setInteractiveMode(true);
    
    std::string line;
    while (true) {
        std::cout << ">>> ";
        std::cout.flush();
        
        if (!std::getline(std::cin, line)) {
            break;
        }
        
        if (line == "exit()" || line == "quit()") {
            break;
        }
        
        // 添加help功能
        if (line == "help()") {
            std::cout << "Welcome to CPPython help utility!" << std::endl;
            std::cout << std::endl;
            std::cout << "Supported features:" << std::endl;
            std::cout << "  - Basic arithmetic operations (+, -, *, /, %)" << std::endl;
            std::cout << "  - Variable assignment (x = 5)" << std::endl;
            std::cout << "  - Print statements (print(\"Hello\"))" << std::endl;
            std::cout << "  - Input function (input(\"prompt\"))" << std::endl;
            std::cout << "  - F-strings (f\"{x}+{y}={x+y}\")" << std::endl;
            std::cout << "  - String operations" << std::endl;
            std::cout << "  - eval() and exec() functions" << std::endl;
            std::cout << "  - Escape sequences (\\n, \\t, \\\\, etc.)" << std::endl;
            std::cout << "  - List operations (one-dimensional and multi-dimensional)" << std::endl;
            std::cout << "  - File operations (open, read, write, with statement)" << std::endl;
            std::cout << "  - Built-in functions (str, int, float, bool, len, repr)" << std::endl;
            std::cout << "  - Comments (# this is a comment)" << std::endl;
            std::cout << std::endl;
            std::cout << "Type \"copyright\", \"credits\" or \"license\" for more information." << std::endl;
            std::cout << "Type \"exit()\" or \"quit()\" to exit." << std::endl;
            continue;
        }
        
        if (line == "help") {
            std::cout << "Type help() for interactive help, or help(object) for help about object." << std::endl;
            continue;
        }
        
        if (line == "copyright") {
            std::cout << "Copyright (c) 2024 CPPython Project. All Rights Reserved." << std::endl;
            continue;
        }
        
        if (line == "credits") {
            std::cout << "    Thanks to Python Software Foundation for inspiration" << std::endl;
            std::cout << "    Thanks to Guido van Rossum for creating Python" << std::endl;
            std::cout << "    Thanks to all contributors to this project" << std::endl;
            continue;
        }
        
        if (line == "license") {
            std::cout << "CPPython is licensed under the MIT License." << std::endl;
            std::cout << "See https://opensource.org/licenses/MIT for more information." << std::endl;
            continue;
        }
        
        if (line.empty()) continue;
        
        try {
            Lexer lexer(line);
            auto tokens = lexer.tokenize();
            
            Parser parser(tokens);
            auto statements = parser.parse();
            
            executor->execute(statements);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

void PythonInterpreter::showHelp() {
    std::cout << "usage: python [option] ... [-c cmd | -m mod | file | -] [arg] ...\n";
    std::cout << "Options and arguments:\n";
    std::cout << "-h, --help     : print this help message and exit\n";
    std::cout << "-v, --version  : print the Python version number and exit\n";
    std::cout << "file           : program read from script file\n";
    std::cout << "-              : program read from stdin\n";
    std::cout << "arg ...        : arguments passed to program in sys.argv[1:]\n";
}

void PythonInterpreter::showVersion() {
    std::cout << "CPPython 1.0.3 (simplified interpreter)" << std::endl;
}