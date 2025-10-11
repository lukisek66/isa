#include "filter.hpp"
#include <fstream>
#include <algorithm>
#include <cctype>

// Pomocné interní funkce — nejsou exportované
static std::string to_lower(const std::string &s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return out;
}

static std::string strip_trailing_dot(const std::string &s) {
    if (!s.empty() && s.back() == '.') {
        return s.substr(0, s.size() - 1);
    }
    return s;
}

// Načtení seznamu blokovaných domén ze souboru
bool load_filter_file(const std::string &filename, std::unordered_set<std::string> &blocked) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Odstranění bílých znaků z obou stran
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Přeskočit prázdné a komentované řádky
        if (line.empty() || line[0] == '#') continue;

        // Normalizace: malá písmena, odstranění koncové tečky
        line = to_lower(strip_trailing_dot(line));

        blocked.insert(line);
    }

    return true;
}

// Ověření, zda je doména blokovaná (včetně poddomén)
bool is_blocked(const std::string &domain, const std::unordered_set<std::string> &blocked) {
    std::string norm = to_lower(strip_trailing_dot(domain));

    // Přesná shoda
    if (blocked.count(norm)) return true;

    // Kontrola poddomén – prochází všechny suffixy
    size_t pos = norm.find('.');
    while (pos != std::string::npos) {
        std::string suffix = norm.substr(pos + 1);
        if (blocked.count(suffix)) return true;
        pos = norm.find('.', pos + 1);
    }

    return false;
}
