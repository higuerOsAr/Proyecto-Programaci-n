#include "ReferenciasGuatemala.h"

#include <algorithm>
#include <cctype>

namespace RefGuatemala {

namespace {

std::string lower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

} // namespace

std::string normalizarMeta(const std::string& meta) {
    const auto m = lower(meta);
    if (m.find("baj") != std::string::npos) {
        return "bajar";
    }
    if (m.find("sub") != std::string::npos || m.find("aument") != std::string::npos) {
        return "subir";
    }
    return "mantener";
}

std::string etiquetaMeta(const std::string& meta) {
    const auto n = normalizarMeta(meta);
    if (n == "bajar") {
        return "Bajar de peso";
    }
    if (n == "subir") {
        return "Subir de peso";
    }
    return "Mantener peso";
}

} // namespace RefGuatemala
