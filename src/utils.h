#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace Utils {
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    bool isNumber(const std::string& str);
    std::string toLower(const std::string& str);
    std::string readFile(const std::string& filename);
    void enableFastIO();
    void throwError(const std::string& message, int line = -1);
}

#endif