#ifndef FILTER_HPP
#define FILTER_HPP

#include <string>
#include <unordered_set>

/**
 * @brief Načte soubor se seznamem blokovaných domén do množiny.
 *
 * @param filename Cesta k souboru s doménami.
 * @param blocked Odkaz na množinu, kam se domény uloží.
 * @return true  Pokud se soubor podařilo načíst.
 * @return false Pokud se soubor nepodařilo otevřít.
 */
bool load_filter_file(const std::string &filename, std::unordered_set<std::string> &blocked);

/**
 * @brief Ověří, zda je daná doména blokovaná (včetně poddomén).
 *
 * @param domain Doména z DNS dotazu (např. "video.ads.google.com").
 * @param blocked Množina blokovaných domén.
 * @return true  Pokud je doména nebo její nadřazená doména v seznamu blokovaných.
 * @return false Jinak.
 */
bool is_blocked(const std::string &domain, const std::unordered_set<std::string> &blocked);

#endif // FILTER_HPP

