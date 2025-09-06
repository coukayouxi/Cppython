#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <memory>

class Executor;

class PythonInterpreter {
private:
    std::unique_ptr<Executor> executor;
    
public:
    PythonInterpreter();
    ~PythonInterpreter();
    
    bool executeFile(const std::string& filename);
    void interactiveMode();
    void showHelp();
    void showVersion();
};

#endif