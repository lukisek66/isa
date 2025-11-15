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
#include <atomic>

/**
 * @brief Informace o čekajícím dotazu.
 *
 * Ukládá adresu klienta, původní ID dotazu a čas registrace.
 */
struct PendingQueryF {
    sockaddr_storage client_addr;
    socklen_t client_len;
    uint16_t original_id;
    int client_fd;
    std::chrono::steady_clock::time_point timestamp;
};



static int resolver_sock = -1;


static sockaddr_storage resolver_addr{};
static socklen_t resolver_addrlen = 0;

//mapování new_id
static std::unordered_map<uint16_t, PendingQueryF> pending;
static std::mutex pending_mtx;

//čítač ID pro generování unikátních ID
static std::atomic<uint16_t> id_counter{1};

//vygeneruje nové ID pro upstream dotaz bez kolizí
uint16_t forwarder_generate_id() {
    uint16_t id;

    while (true) {
        id = id_counter.fetch_add(1);

        if (id == 0) {      // přetečení → nastavíme zpět na 1
            id_counter = 1;
            id = 1;
        }

        std::lock_guard<std::mutex> g(pending_mtx);
        if (pending.count(id) == 0)
            return id;      // ID je volné → použijeme
    }
}

//inicializuje forwarder: vytvoří socket a uloží adresu resolveru
bool forwarder_init(const std::string &server) {
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int rc = getaddrinfo(server.c_str(), "53", &hints, &res);
    if (rc != 0) {
        print_error(std::string("forwarder_init: ") + gai_strerror(rc));
        return false;
    }

    for (auto *p = res; p != nullptr; p = p->ai_next) {
        resolver_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (resolver_sock < 0) continue;

        memcpy(&resolver_addr, p->ai_addr, p->ai_addrlen);
        resolver_addrlen = p->ai_addrlen;
        break;
    }

    freeaddrinfo(res);

    if (resolver_sock < 0) {
        print_error("forwarder_init: socket selhal");
        return false;
    }

    return true;
}


//vrátí file descriptor socketu směrovače
int forwarder_get_fd() {
    return resolver_sock;
}


//uloží mapping new_id a původní dotaz a odešle paket upstreamu
bool forwarder_send_and_register(const uint8_t *buf, size_t len,
                                 uint16_t new_id,
                                 const sockaddr_storage &client_addr,
                                 socklen_t client_len,
                                 uint16_t original_id,
                                 int client_fd)
{
    if (resolver_sock < 0) return false;

    PendingQueryF pq;
    pq.client_addr = client_addr;
    pq.client_len = client_len;
    pq.original_id = original_id;
    pq.client_fd = client_fd; // socket, přes který dotaz přišel
    pq.timestamp = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> g(pending_mtx);
        pending[new_id] = pq;
    }

    ssize_t sent = sendto(resolver_sock, buf, len, 0,
                          (sockaddr*)&resolver_addr, resolver_addrlen);

    if (sent < 0) {
        std::lock_guard<std::mutex> g(pending_mtx);
        pending.erase(new_id);
        return false;
    }

    return true;
}


//pomocné funkce pro čtení/zápis 16bit hodnot v bufferu
static inline uint16_t buf_get_u16(const uint8_t *p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

static inline void buf_set_u16(uint8_t *p, uint16_t v) {
    p[0] = (v >> 8) & 0xFF;
    p[1] = v & 0xFF;
}

//zpracuje odpověď z upstreamu a pošle ji klientovi
bool forwarder_handle_response(uint8_t *buf, ssize_t len, int client_sock) {
    if (len < 12) return false; // Minimální délka DNS paketu

    uint16_t upstream_id = buf_get_u16(buf);
    PendingQueryF pq;

    {
        std::lock_guard<std::mutex> g(pending_mtx);
        auto it = pending.find(upstream_id);

        if (it == pending.end())
            return false;

        pq = it->second;
        //odstranit mapping, jinak dochází k úniku
        pending.erase(it);
    }

    //obnovit původní ID klienta
    buf_set_u16(buf, pq.original_id);

    //odeslat klientovi
    ssize_t sent = sendto(pq.client_fd, buf, len, 0,
                      (sockaddr*)&pq.client_addr, pq.client_len);


    if (sent < 0) {
        print_error("forwarder_handle_response: sendto klientovi selhalo");
        return false;
    }

    /*
    std::cout << "[FORWARDER] Přijal odpověď ID=" << upstream_id
              << ", vracím klientovi ID=" << pq.original_id
              << std::endl;
    */

    return true;
}
