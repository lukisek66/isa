#ifndef FORWARDER_HPP
#define FORWARDER_HPP

#include <cstdint>
#include <cstddef>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <vector>

/**
 * @brief Inicializuje forwarder pro komunikaci s upstream DNS serverem.
 *
 * Přeloží hostname/IP serveru, otevře socket pro UDP komunikaci s upstreamem
 * a připraví interní struktury pro mapování ID dotazů.
 *
 * @param server Hostname nebo IP upstream DNS serveru
 * @return true pokud byla inicializace úspěšná, false pokud došlo k chybě
 */
bool forwarder_init(const std::string &server);

/**
 * @brief Vrací file descriptor socketu forwarderu.
 *
 * Tento socket poslouchá odpovědi od upstream DNS serveru.
 * Je vhodné jej přidat do `select()` nebo `poll()` pro asynchronní čtení.
 *
 * @return FD socketu pokud je forwarder inicializován, jinak -1
 */
int forwarder_get_fd();

/**
 * @brief Vygeneruje nové náhodné ID pro DNS dotaz.
 *
 * Používá se pro přemapování ID klientských dotazů, aby bylo možné sledovat odpovědi od upstreamu.
 *
 * @return Nově vygenerované 16-bitové ID
 */
uint16_t forwarder_generate_id();

/**
 * @brief Odešle DNS paket na upstream resolver a zaregistruje mapování ID.
 *
 * Funkce přemapuje původní ID dotazu klienta na nové ID (`new_id`), odešle paket
 * na upstream server a uloží mapování, aby bylo možné odpověď zpětně doručit klientovi.
 *
 * @param buf Ukazatel na buffer obsahující DNS paket
 * @param len Délka bufferu
 * @param new_id Nové ID pro tento dotaz
 * @param client_addr Adresa klienta, na kterého se má odpověď poslat
 * @param client_len Délka adresy klienta
 * @param original_id Původní ID dotazu od klienta
 * @param client_fd číslo soketu
 * @return true pokud `sendto()` úspěšně odeslal paket a mapování bylo zaregistrováno, false jinak
 */
bool forwarder_send_and_register(const uint8_t *buf, size_t len,
                                 uint16_t new_id,
                                 const sockaddr_storage &client_addr,
                                 socklen_t client_len,
                                 uint16_t original_id,
                                 int client_fd);

/**
 * @brief Zpracuje odpověď od upstream DNS serveru.
 *
 * Funkce kontroluje, zda odpověď patří zaregistrovanému dotazu,
 * obnoví původní ID dotazu a pošle paket zpět klientovi.
 *
 * @param buf Buffer obsahující obdržený DNS paket
 * @param len Délka bufferu
 * @return true pokud byla odpověď úspěšně doručena klientovi, false jinak
 */
bool forwarder_handle_response(uint8_t *buf, ssize_t len);

#endif // FORWARDER_HPP
