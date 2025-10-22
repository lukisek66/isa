#include "forwarder.hpp"
#include "util.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <random>

struct PendingQueryF {
    sockaddr_storage client_addr;
    socklen_t client_len;
    uint16_t original_id;
    std::chrono::steady_clock::time_point timestamp;
};

static int resolver_sock = -1;
static sockaddr_storage resolver_addr{};
static socklen_t resolver_addrlen = 0;
static std::unordered_map<uint16_t, PendingQueryF> pending;
static std::mutex pending_mtx;

// Random generator for IDs (non-crypto, good enough here)
static std::mt19937 rng((unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count());
static std::uniform_int_distribution<uint32_t> id_distr(1, 0xFFFF);

bool forwarder_init(const std::string &server, int port) {
    if (server.empty()) {
        print_error("forwarder_init: prázdný server");
        return false;
    }

    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int rc = getaddrinfo(server.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (rc != 0) {
        print_error(std::string("forwarder_init: getaddrinfo: ") + gai_strerror(rc));
        return false;
    }

    // vytvoříme socket podle první vhodné adresy (pokud je více, bereme tu první)
    struct addrinfo *p;
    for (p = res; p != nullptr; p = p->ai_next) {
        resolver_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (resolver_sock < 0) continue;

        // nemusíme bindovat konkrétní adresu, OS vybere port
        // u IPv6 dovolíme dual-stack, pokud systém podporuje (nepřepisovat IPV6_V6ONLY)
        // nastavíme non-blocking? Ne, necháme blocking a dns_server použije select.
        memcpy(&resolver_addr, p->ai_addr, p->ai_addrlen);
        resolver_addrlen = p->ai_addrlen;
        break;
    }

    freeaddrinfo(res);

    if (resolver_sock < 0) {
        print_error("forwarder_init: nepodařilo se vytvořit socket pro resolver");
        return false;
    }

    return true;
}

int forwarder_get_fd() {
    return resolver_sock;
}

bool forwarder_send_and_register(const uint8_t *buf, size_t len,
                                 uint16_t new_id,
                                 const sockaddr_storage &client_addr,
                                 socklen_t client_len,
                                 uint16_t original_id)
{
    if (resolver_sock < 0) return false;

    // uložíme mapu
    PendingQueryF pq;
    pq.client_addr = client_addr;
    pq.client_len = client_len;
    pq.original_id = original_id;
    pq.timestamp = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> g(pending_mtx);
        pending[new_id] = pq;
    }

    // pošleme na resolver
    ssize_t sent = sendto(resolver_sock, buf, len, 0,
                          (struct sockaddr *)&resolver_addr, resolver_addrlen);
    if (sent < 0) {
        std::lock_guard<std::mutex> g(pending_mtx);
        pending.erase(new_id);
        return false;
    }

    return true;
}

static inline uint16_t buf_get_u16(const uint8_t *p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}
static inline void buf_set_u16(uint8_t *p, uint16_t v) {
    p[0] = (v >> 8) & 0xFF;
    p[1] = v & 0xFF;
}

bool forwarder_handle_response(uint8_t *buf, ssize_t len, int client_sock) {
    if (len < (ssize_t)sizeof(uint16_t) * 6) return false; // min header
    uint8_t *hdr = buf;
    uint16_t resp_id = buf_get_u16(hdr);

    PendingQueryF pq;
    {
        std::lock_guard<std::mutex> g(pending_mtx);
        auto it = pending.find(resp_id);
        if (it == pending.end()) {
            // Neznámé ID — ignoruj
            return false;
        }
        pq = it->second;
        pending.erase(it);
    }

    // obnovíme původní ID (uloženo v pq.original_id)
    buf_set_u16(hdr, pq.original_id);

    // pošleme odpověď klientovi (client_sock je socket na kterém server poslouchá, obvykle ten samý sockfd)
    ssize_t sent = sendto(client_sock, buf, len, 0,
                          (struct sockaddr *)&pq.client_addr, pq.client_len);
    if (sent < 0) {
        print_error("forwarder_handle_response: sendto klientovi selhalo");
        return false;
    }
    std::cout << "[FORWARDER] Přijal odpověď ID=" << resp_id
          << ", vracím klientovi ID=" << pq.original_id
          << std::endl;


    return true;
}
