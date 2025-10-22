#include "filter.hpp"
#include "util.hpp"
#include "dns_server.hpp"
#include "forwarder.hpp"

#include <iostream>
#include <string>
#include <unordered_set>
#include <cstdlib>
#include <netdb.h>
#include <cstring>

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
// Překlad adresy resolveru (IPv4/IPv6 nebo jméno)
// -----------------------------------------------------------
bool resolve_server_address(const std::string &host, int port,
                            sockaddr_storage &out_addr, socklen_t &out_len)
{
    addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int ret = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (ret != 0) {
        print_error("Nepodařilo se přeložit adresu resolveru: " + std::string(gai_strerror(ret)));
        return false;
    }

    memcpy(&out_addr, res->ai_addr, res->ai_addrlen);
    out_len = res->ai_addrlen;
    freeaddrinfo(res);
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

    std::unordered_set<std::string> blocked;
    if (!load_filter_file(cfg.filter_file, blocked)) {
        print_error("Nepodařilo se načíst soubor s blokovanými doménami: " + cfg.filter_file);
        return EXIT_FAILURE;
    }

    print_info("Načteno " + std::to_string(blocked.size()) + " blokovaných domén.", cfg.verbose);

    sockaddr_storage resolver_addr{};
    socklen_t resolver_addrlen = 0;
    if (!resolve_server_address(cfg.server, cfg.port, resolver_addr, resolver_addrlen)) {
        return EXIT_FAILURE;
    }

    // Testovací výpis
    //------------------------------------------------------------
    std::string test_domains[] = {
        "ads.google.com", "example.com", "bad.site",
        "tracker.example.org", "cdn.tracker.example.org"
    };

    for (const auto &d : test_domains) {
        bool blk = is_blocked(d, blocked);
        std::cout << d << " -> " << (blk ? "BLOCKED" : "OK") << std::endl;
    }
    //------------------------------------------------------------

    if (!forwarder_init(cfg.server, cfg.port)) {
    print_error("Nelze inicializovat forwarder na " + cfg.server);
    return EXIT_FAILURE;
    }


    // Spuštění DNS serveru (IPv4 + IPv6)
    start_dns_server("::", cfg.port, blocked, cfg.verbose);
    return EXIT_SUCCESS;
}
