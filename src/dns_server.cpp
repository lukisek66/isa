#include "dns_server.hpp"
#include "util.hpp"
#include "filter.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

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

static std::string parse_qname(const uint8_t *data, size_t len, size_t &offset) {
    std::string name;
    while (offset < len) {
        uint8_t label_len = data[offset++];
        if (label_len == 0)
            break;
        if (offset + label_len > len)
            return "<invalid>";
        if (!name.empty()) name += ".";
        name.append(reinterpret_cast<const char*>(data + offset), label_len);
        offset += label_len;
    }
    return name;
}

bool start_dns_server(const std::string &listen_addr, int port,
                      const std::unordered_set<std::string> &blocked,
                      bool verbose) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        print_error("Nepodařilo se vytvořit socket.");
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        print_error("Nepodařilo se bindnout socket. Zkuste jiný port (např. -p 8053).");
        close(sockfd);
        return false;
    }

    print_info("DNS server běží na portu " + std::to_string(port), verbose);

    std::vector<uint8_t> buffer(512);

    while (true) {
        sockaddr_in client{};
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
    }

    close(sockfd);
    return true;
}
