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
#include <csignal>
#include <atomic>

/**
 * @brief Globální flag pro běh programu.
 *
 * Slouží k indikaci, zda má hlavní smyčka běžet.
 * Nastavuje se na false při zachycení SIGINT (Ctrl+C).
 */
std::atomic<bool> running{true};

/**
 * @brief Handler pro signál SIGINT.
 *
 * Nastaví flag `running` na false, aby se hlavní smyčka programu bezpečně ukončila.
 *
 * @param signo Číslo signálu (není využito)
 */
void handle_sigint(int) {
    running = false;
}

/**
 * @brief Konfigurace programu z příkazové řádky.
 *
 * Obsahuje IP/hostname serveru, port, soubor s filtrem a verbose mód.
 */
struct Config {
    std::string server;      /**< IP adresa nebo hostname resolveru */
    int port = 53;           /**< Port resolveru, defaultně 53 */
    std::string filter_file; /**< Cesta k souboru s blokovanými doménami */
    bool verbose = false;    /**< Flag pro zapnutí detailního výpisu */
};

/**
 * @brief Parsuje argumenty příkazové řádky.
 *
 * Podporované argumenty:
 * - `-s <server>` : nastaví IP/hostname resolveru
 * - `-p <port>`   : port dns filtru (volitelně, default 53)
 * - `-f <file>`   : cesta k souboru s blokovanými doménami
 * - `-v`          : zapnutí verbose módu
 *
 * @param argc Počet argumentů
 * @param argv Pole argumentů
 * @param cfg Výstupní struktura Config
 * @return true pokud jsou argumenty validní, false jinak
 */
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

/**
 * @brief Přeloží hostname/IP adresu a port na sockaddr_storage.
 *
 * Podporuje IPv4 i IPv6 adresy.
 *
 * @param host Hostname nebo IP resolveru
 * @param port __
 * @param out_addr Výstupní adresa (sockaddr_storage)
 * @param out_len Délka výstupní adresy
 * @return true pokud byl překlad úspěšný, false jinak
 */
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

/**
 * @brief Hlavní entry point programu.
 *
 * Inicializuje signal handler, načte konfiguraci a filtr, přeloží resolver adresu,
 * inicializuje forwarder a spustí DNS server.
 *
 * @param argc Počet argumentů
 * @param argv Pole argumentů
 * @return EXIT_SUCCESS při úspěšném běhu, EXIT_FAILURE při chybě
 */
int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
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

    if (!forwarder_init(cfg.server)) {
        print_error("Nelze inicializovat forwarder na " + cfg.server);
        return EXIT_FAILURE;
    }

    // Spuštění DNS serveru (IPv4 + IPv6)
    start_dns_server(cfg.port, blocked, cfg.verbose);
    return EXIT_SUCCESS;
}
