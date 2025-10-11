#include "dns_server.hpp"
#include "util.hpp"
#include "filter.hpp"


#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

// DNS hlavička
/*
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
*/
// -----------------------------------------------------------
// Parsování QNAME z DNS dotazu (včetně základní podpory komprese)
// -----------------------------------------------------------
static std::string parse_qname(const uint8_t *data, size_t len, size_t &offset) {
    std::string name;
    size_t jumped_offset = 0;
    bool jumped = false;

    while (offset < len) {
        uint8_t label_len = data[offset];

        // 0xC0 = kompresní pointer (RFC1035 4.1.4)
        if ((label_len & 0xC0) == 0xC0) {
            if (offset + 1 >= len)
                return "<invalid compression>";
            uint16_t pointer = ((label_len & 0x3F) << 8) | data[offset + 1];
            if (!jumped) {
                jumped_offset = offset + 2;
                jumped = true;
            }
            offset = pointer;
            continue;
        }

        if (label_len == 0) {
            offset++;
            break;
        }

        if (offset + 1 + label_len > len)
            return "<invalid>";

        if (!name.empty()) name += ".";
        name.append(reinterpret_cast<const char*>(data + offset + 1), label_len);
        offset += label_len + 1;
    }

    if (jumped)
        offset = jumped_offset;

    return name;
}


// ---------------------------------------------------------------
// Příprava odpovědi při zjištění blokované domény, či né typu A
// ---------------------------------------------------------------
std::vector<uint8_t> build_error_response(const DNSHeader *hdr, const uint8_t *question, size_t qlen, uint16_t rcode) {
    std::vector<uint8_t> resp(sizeof(DNSHeader) + qlen);

    // kopíruj hlavičku a uprav flags
    DNSHeader *res_hdr = reinterpret_cast<DNSHeader *>(resp.data());
    *res_hdr = *hdr;

    // Nastav QR (response), OPCODE stejný, RCODE = rcode
    uint16_t flags = ntohs(hdr->flags);
    flags |= 0x8000;         // QR = 1
    flags &= 0xFFF0;         // vymaž starý RCODE
    flags |= rcode & 0x000F; // nastav nový RCODE
    res_hdr->flags = htons(flags);

    // QDCOUNT = 1 (pokud byla otázka 1), AN/NS/AR = 0
    res_hdr->ancount = 0;
    res_hdr->nscount = 0;
    res_hdr->arcount = 0;

    // Kopíruj Question sekci
    std::memcpy(resp.data() + sizeof(DNSHeader), question, qlen);

    return resp;
}

// -----------------------------------------------------------
// Inicializace UDP socketů (IPv4 i IPv6) a příjem dotazů
// -----------------------------------------------------------
bool start_dns_server(const std::string &listen_addr, int port,
                      const std::unordered_set<std::string> &blocked,
                      bool verbose) {
    struct addrinfo hints{}, *res, *p;
    int sockfd = -1;

    hints.ai_family = AF_UNSPEC;      // IPv4 i IPv6
    hints.ai_socktype = SOCK_DGRAM;   // UDP
    hints.ai_flags = AI_PASSIVE;      // naslouchací socket

    std::string port_str = std::to_string(port);
    int status = getaddrinfo(listen_addr.empty() ? nullptr : listen_addr.c_str(),
                             port_str.c_str(), &hints, &res);
    if (status != 0) {
        print_error("getaddrinfo selhalo: " + std::string(gai_strerror(status)));
        return false;
    }

    // Zkusíme bindnout první funkční adresu
    for (p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) continue;

        int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);

    if (sockfd < 0) {
        print_error("Nepodařilo se bindnout socket (zkus jiný port např. -p 8053).");
        return false;
    }

    print_info("DNS server běží na portu " + std::to_string(port), verbose);

    std::vector<uint8_t> buffer(512);

    while (true) {
        sockaddr_storage client{};
        socklen_t client_len = sizeof(client);
        ssize_t n = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                             (sockaddr *)&client, &client_len);
        if (n <= 0) continue;

        if ((size_t)n < sizeof(DNSHeader)) {
            print_error("Příchozí paket je příliš krátký.");
            continue;
        }

        const DNSHeader *hdr = reinterpret_cast<const DNSHeader *>(buffer.data());
        size_t offset = sizeof(DNSHeader);

        std::string qname = parse_qname(buffer.data(), n, offset);

        if (offset + 4 > (size_t)n) {
            print_error("Neplatná DNS otázka.");
            continue;
        }

        uint16_t qtype = ntohs(*(uint16_t *)(buffer.data() + offset));
        offset += 2;
        uint16_t qclass = ntohs(*(uint16_t *)(buffer.data() + offset));
        offset += 2;

        if (verbose) {
            std::cout << "[DNS] Dotaz ID=" << ntohs(hdr->id)
                      << " jméno=" << qname
                      << " typ=" << qtype
                      << " class=" << qclass << std::endl;
        }

        // Najdi začátek Question sekce pro kopírování
        size_t qlen = offset - sizeof(DNSHeader);

        // Pokud typ dotazu není A
        if (qtype != 1) {
            auto resp = build_error_response(hdr, buffer.data() + sizeof(DNSHeader), qlen, 4); // NOTIMP
            sendto(sockfd, resp.data(), resp.size(), 0, (sockaddr *)&client, client_len);
            if (verbose) std::cout << "[DNS] Odesláno NOTIMP" << std::endl;
            continue;
        }

        // Pokud doména je blokovaná
        if (is_blocked(qname, blocked)) {
            auto resp = build_error_response(hdr, buffer.data() + sizeof(DNSHeader), qlen, 5); // REFUSED
            sendto(sockfd, resp.data(), resp.size(), 0, (sockaddr *)&client, client_len);
            if (verbose) std::cout << "[DNS] Odesláno REFUSED" << std::endl;
            continue;
        }

        // Zde bude v budoucnu forwarding pro povolené dotazy typu A
    }

    close(sockfd);
    return true;
}
