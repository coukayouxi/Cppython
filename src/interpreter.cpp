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
    std::cout << "Python 3.11.0 (simplified interpreter)" << std::endl;
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
    std::cout << "Python 3.11.0 (simplified interpreter)" << std::endl;
}