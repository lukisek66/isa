#ifndef DNS_SERVER_HPP
#define DNS_SERVER_HPP

#include <vector>   // std::vector
#include <cstdint>  // uint8_t, uint16_t
#include <string>
#include <unordered_set>

// --- DNS hlavička ---
#pragma pack(push, 1)
struct DNSHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};
#pragma pack(pop)

// --- Funkce ---
bool start_dns_server(const std::string &listen_addr, int port,
                      const std::unordered_set<std::string> &blocked,
                      bool verbose);

// Vytvoření chybové odpovědi (REFUSED / NOTIMP)
std::vector<uint8_t> build_error_response(const DNSHeader *hdr,
                                          const uint8_t *question, size_t qlen,
                                          uint16_t rcode);

#endif // DNS_SERVER_HPP

