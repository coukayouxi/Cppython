#include "interpreter.h"
#include "utils.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // 启用快速IO
    Utils::enableFastIO();
    
    PythonInterpreter interpreter;
    
    if (argc == 1) {
        // 交互模式
        interpreter.interactiveMode();
    } else if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "-h" || arg == "--help") {
            interpreter.showHelp();
        } else if (arg == "-v" || arg == "--version") {
            interpreter.showVersion();
        } else {
            // 执行Python文件
            if (!interpreter.executeFile(arg)) {
                return 1;
            }
        }
    } else {
        std::cerr << "Usage: " << argv[0] << " [script.py] [-h|--help] [-v|--version]" << std::endl;
        return 1;
    }
    
    return 0;
}