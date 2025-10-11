#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <iostream>

inline void print_error(const std::string &msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}

inline void print_info(const std::string &msg, bool verbose) {
    if (verbose)
        std::cerr << "[INFO] " << msg << std::endl;
}

#endif
