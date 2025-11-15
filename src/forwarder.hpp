#ifndef FORWARDER_HPP
#define FORWARDER_HPP

#include <cstdint>
#include <cstddef>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

#include <vector>

// Inicializace forwarderu: přeloží cfg.server a otevře socket pro upstream
// Vrací true pokud OK.
bool forwarder_init(const std::string &server);

// Vrátí fd socketu, který poslouchá odpovědi od upstreamu (přidat do select()).
// Pokud není inicializován, vrací -1.
int forwarder_get_fd();

uint16_t forwarder_generate_id();

// Odešle paket na resolver a zaregistruje mapování new_id -> klient.
// Vrací true pokud sendto() úspěšně odeslal paket (mapování je zaregistrováno).
bool forwarder_send_and_register(const uint8_t *buf, size_t len,
                                 uint16_t new_id,
                                 const sockaddr_storage &client_addr,
                                 socklen_t client_len,
                                 uint16_t original_id);

// Zpracuje odpověď obdrženou z upstreamu (buffer obsahuje paket o délce len).
// Pokud odpověď patří zaregistrovanému dotazu, obnoví původní ID a pošle ji na client_sock.
// Vrací true pokud byla odpověď předána klientovi.
bool forwarder_handle_response(uint8_t *buf, ssize_t len, int client_sock);

#endif // FORWARDER_HPP
