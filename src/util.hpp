#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <iostream>

/**
 * @brief Vypíše chybovou zprávu na standardní chybový výstup.
 *
 * @param msg Text chybové zprávy.
 *
 * Tento inline helper slouží k jednotnému formátování chybových zpráv
 * ve formě: [ERROR] zpráva.
 */
inline void print_error(const std::string &msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}

/**
 * @brief Vypíše informativní zprávu na standardní chybový výstup, pokud je zapnutý verbose režim.
 *
 * @param msg Text informační zprávy.
 * @param verbose Pokud je true, zpráva se vypíše, jinak se ignoruje.
 *
 * Tento inline helper je vhodný pro ladění nebo zobrazování doplňujících informací,
 * které nejsou kritické pro běh programu.
 */
inline void print_info(const std::string &msg, bool verbose) {
    if (verbose)
        std::cerr << "[INFO] " << msg << std::endl;
}

#endif

