#ifndef DNS_SERVER_HPP
#define DNS_SERVER_HPP

#include <vector>
#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_set>

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

bool start_dns_server(const std::string &listen_addr, int port,
                      const std::unordered_set<std::string> &blocked,
                      bool verbose);

// pomocná pro tvorbu chybových odpovědí
std::vector<uint8_t> build_error_response(const DNSHeader *hdr,
                                          const uint8_t *question, size_t qlen,
                                          uint16_t rcode);

#endif // DNS_SERVER_HPP
