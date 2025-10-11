#include "filter.hpp"
#include "util.hpp"
#include <iostream>
#include <string>
#include <unordered_set>
#include <cstdlib>
#include "dns_server.hpp"

// -----------------------------------------------------------
// Struktura konfigurace
// -----------------------------------------------------------
struct Config {
    std::string server;
    int port = 53;
    std::string filter_file;
    bool verbose = false;
};

// -----------------------------------------------------------
// Zpracování argumentů příkazové řádky
// -----------------------------------------------------------
bool parse_args(int argc, char *argv[], Config &cfg) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s" && i + 1 < argc) {
            cfg.server = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            cfg.port = std::stoi(argv[++i]);
        } else if (arg == "-f" && i + 1 < argc) {
            cfg.filter_file = argv[++i];
        } else if (arg == "-v") {
            cfg.verbose = true;
        } else {
            print_error("Neznámý nebo nekompletní argument: " + arg);
            return false;
        }
    }

    if (cfg.server.empty() || cfg.filter_file.empty()) {
        print_error("Chybí povinné argumenty -s <server> a -f <filter_file>");
        return false;
    }

    return true;
}

// -----------------------------------------------------------
// Hlavní funkce
// -----------------------------------------------------------
int main(int argc, char *argv[]) {
    Config cfg;
    if (!parse_args(argc, argv, cfg)) {
        std::cerr << "Použití: dns -s server [-p port] -f filter_file [-v]\n";
        return EXIT_FAILURE;
    }

    // --- Načtení blokovaných domén ---
    std::unordered_set<std::string> blocked;
    if (!load_filter_file(cfg.filter_file, blocked)) {
        print_error("Nepodařilo se načíst soubor s blokovanými doménami: " + cfg.filter_file);
        return EXIT_FAILURE;
    }

    print_info("Načteno " + std::to_string(blocked.size()) + " blokovaných domén.", cfg.verbose);

    // --- Testovací část (zatím) ---
    std::string test_domains[] = {
        "ads.google.com",
        "example.com",
        "bad.site",
        "tracker.example.org",
        "cdn.tracker.example.org"
    };

    for (const auto &d : test_domains) {
        bool blk = is_blocked(d, blocked);
        std::cout << d << " -> " << (blk ? "BLOCKED" : "OK") << std::endl;
    }

    // --- Spuštění DNS serveru ---
    start_dns_server("0.0.0.0", cfg.port, blocked, cfg.verbose);


    return EXIT_SUCCESS;
}
