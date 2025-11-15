#include "dns_server.hpp"
#include "util.hpp"
#include "filter.hpp"
#include "forwarder.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <mutex>
#include <cstdlib>
#include <random>
#include <atomic>

extern std::atomic<bool> running;

// --- parse_qname ---
static std::string parse_qname(const uint8_t *data, size_t len, size_t &offset) {
    std::string name;
    size_t jumped_offset = 0;
    bool jumped = false;

    while (offset < len) {
        uint8_t label_len = data[offset];
        if ((label_len & 0xC0) == 0xC0) {
            if (offset + 1 >= len) return "<invalid compression>";
            uint16_t pointer = ((label_len & 0x3F) << 8) | data[offset + 1];
            if (!jumped) { jumped_offset = offset + 2; jumped = true; }
            offset = pointer;
            continue;
        }
        if (label_len == 0) { offset++; break; }
        if (offset + 1 + label_len > len) return "<invalid>";
        if (!name.empty()) name += ".";
        name.append(reinterpret_cast<const char*>(data + offset + 1), label_len);
        offset += label_len + 1;
    }
    if (jumped) offset = jumped_offset;
    return name;
}

// --- build_error_response ---
std::vector<uint8_t> build_error_response(const DNSHeader *hdr, const uint8_t *question, size_t qlen, uint16_t rcode) {
    std::vector<uint8_t> resp(sizeof(DNSHeader) + qlen);
    DNSHeader *res_hdr = reinterpret_cast<DNSHeader *>(resp.data());
    *res_hdr = *hdr;
    uint16_t flags = ntohs(hdr->flags);
    flags |= 0x8000;         // QR = 1
    flags &= 0xFFF0;         // clear old RCODE
    flags |= rcode & 0x000F; // new RCODE
    res_hdr->flags = htons(flags);
    res_hdr->ancount = 0;
    res_hdr->nscount = 0;
    res_hdr->arcount = 0;
    std::memcpy(resp.data() + sizeof(DNSHeader), question, qlen);
    return resp;
}

bool start_dns_server(const std::string &listen_addr, int port,
                      const std::unordered_set<std::string> &blocked,
                      bool verbose) {
    struct addrinfo hints{}, *res, *p;
    int client_sock = -1;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    std::string port_str = std::to_string(port);
    int status = getaddrinfo(listen_addr.empty() ? nullptr : listen_addr.c_str(),
                             port_str.c_str(), &hints, &res);
    if (status != 0) {
        print_error("getaddrinfo selhalo: " + std::string(gai_strerror(status)));
        return false;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        client_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (client_sock < 0) continue;
        int yes = 1;
        setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        if (bind(client_sock, p->ai_addr, p->ai_addrlen) == 0) break;
        close(client_sock);
        client_sock = -1;
    }
    freeaddrinfo(res);

    if (client_sock < 0) {
        print_error("Nepodařilo se bindnout klientský socket.");
        return false;
    }

    int ffd = forwarder_get_fd();
    if (ffd < 0)
        print_info("Forwarder není inicializován — dotazy budou jen logovány.", verbose);

    print_info("DNS server běží na portu " + std::to_string(port), verbose);

    std::vector<uint8_t> buffer(4096);

    while (running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(client_sock, &fds);
        int maxfd = client_sock;
        if (ffd >= 0) {
            FD_SET(ffd, &fds);
            if (ffd > maxfd) maxfd = ffd;
        }

        struct timeval tv{};
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ready = select(maxfd + 1, &fds, nullptr, nullptr, &tv);
        if (ready < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (!running) break;

        // 1️⃣ odpověď z upstream resolveru
        if (ffd >= 0 && FD_ISSET(ffd, &fds)) {
            sockaddr_storage from{};
            socklen_t from_len = sizeof(from);
            ssize_t rlen = recvfrom(ffd, buffer.data(), buffer.size(), 0,
                                    (sockaddr*)&from, &from_len);
            if (rlen > 0) {
                bool ok = forwarder_handle_response(buffer.data(), rlen, client_sock);
                if (verbose && ok)
                    std::cout << "[FORWARDER] Odpověď přeposlána klientovi" << std::endl;
            }
        }

        // 2️⃣ dotaz od klienta
        if (FD_ISSET(client_sock, &fds)) {
            sockaddr_storage client{};
            socklen_t client_len = sizeof(client);
            ssize_t n = recvfrom(client_sock, buffer.data(), buffer.size(), 0,
                                 (sockaddr*)&client, &client_len);
            if (n <= 0) continue;
            if ((size_t)n < sizeof(DNSHeader)) continue;

            const DNSHeader *hdr = reinterpret_cast<const DNSHeader*>(buffer.data());
            size_t offset = sizeof(DNSHeader);
            std::string qname = parse_qname(buffer.data(), n, offset);
            if (offset + 4 > (size_t)n) continue;

            uint16_t qtype = ntohs(*(uint16_t*)(buffer.data() + offset));
            offset += 2;
            uint16_t qclass = ntohs(*(uint16_t*)(buffer.data() + offset));
            offset += 2;

            if (verbose)
                std::cout << "[DNS] Dotaz ID=" << ntohs(hdr->id)
                          << " jméno=" << qname
                          << " typ=" << qtype
                          << " class=" << qclass << std::endl;

            size_t qlen = offset - sizeof(DNSHeader);

            if (qtype != 1) {
                auto resp = build_error_response(hdr, buffer.data() + sizeof(DNSHeader), qlen, 4);
                sendto(client_sock, resp.data(), resp.size(), 0, (sockaddr*)&client, client_len);
                if (verbose){
                    std::cout << "Wrong type" << std::endl;
                }
                continue;
            }

            if (is_blocked(qname, blocked)) {
                auto resp = build_error_response(hdr, buffer.data() + sizeof(DNSHeader), qlen, 5);
                sendto(client_sock, resp.data(), resp.size(), 0, (sockaddr*)&client, client_len);
                if (verbose){
                    std::cout << "Blocked" << std::endl;
                }
                continue;
            }

            if (ffd < 0) continue;

            uint16_t original_id = ntohs(hdr->id);
            uint16_t new_id = forwarder_generate_id();     // <- nové deterministické ID

            buffer[0] = (new_id >> 8) & 0xFF;
            buffer[1] = new_id & 0xFF;


            bool ok = forwarder_send_and_register(buffer.data(), n, new_id, client, client_len, original_id);
            if (verbose && ok)
                std::cout << "[FORWARD] Dotaz přeposlán upstream (ID=" << new_id << ")" << std::endl;
        }
    }

    close(client_sock);
    if (ffd >= 0) close(ffd);
    print_info("Server ukončen.", verbose);
    return true;
}
