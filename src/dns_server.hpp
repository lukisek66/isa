#ifndef DNS_SERVER_HPP
#define DNS_SERVER_HPP

#include <vector>
#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_set>

/**
 * @brief Struktura hlavičky DNS paketu.
 *
 * Používá se pro čtení a zápis základních informací o DNS dotazu nebo odpovědi.
 * Struktura je zarovnána na 1 byte (`#pragma pack(1)`), aby odpovídala síťovému formátu.
 */
#pragma pack(push, 1)
struct DNSHeader {
    uint16_t id;       /**< Identifikátor DNS dotazu */
    uint16_t flags;    /**< Vlajky (QR, OPCODE, AA, TC, RD, RA, RCODE) */
    uint16_t qdcount;  /**< Počet položek v otázce */
    uint16_t ancount;  /**< Počet odpovědí */
    uint16_t nscount;  /**< Počet autoritativních záznamů */
    uint16_t arcount;  /**< Počet záznamů v dalších sekcích */
};
#pragma pack(pop)

/**
 * @brief Spustí jednoduchý DNS server.
 *
 * Server poslouchá na zadané adrese a portu, kontroluje dotazy proti seznamu blokovaných domén
 * a předává dotazy na forwarder. Podporuje IPv4 i IPv6.
 *
 * @param port Port, na kterém server naslouchá
 * @param blocked Set blokovaných domén, pro které bude server generovat NXDOMAIN nebo chybu
 * @param verbose Pokud je true, vypisuje detailní informace o dotazech a odpovědích
 * @return true pokud se server úspěšně spustil, false při chybě
 */
bool start_dns_server(int port,
                      const std::unordered_set<std::string> &blocked,
                      bool verbose);

/**
 * @brief Vytvoří DNS odpověď s chybovým kódem.
 *
 * Tato pomocná funkce je použita pro odpovědi klientům, když je dotaz neplatný
 * nebo doména je zablokovaná.
 *
 * @param hdr Pointer na hlavičku původního DNS dotazu
 * @param question Pointer na otázku v DNS dotazu
 * @param qlen Délka otázky v bajtech
 * @param rcode RCODE chybového stavu
 * @return Vector bajtů obsahující připravený DNS paket s chybou
 */
std::vector<uint8_t> build_error_response(const DNSHeader *hdr,
                                          const uint8_t *question, size_t qlen,
                                          uint16_t rcode);

#endif // DNS_SERVER_HPP
