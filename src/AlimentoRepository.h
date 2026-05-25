#pragma once

#include "RegistroAlimento.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class Database;

class AlimentoRepository {
public:
    explicit AlimentoRepository(Database& db);

    void createSchema();
    std::int64_t insert(const RegistroAlimento& r);
    std::optional<RegistroAlimento> findById(std::int64_t id);
    std::vector<RegistroAlimento> findByPersonaAndFecha(std::int64_t personaId, const std::string& fecha);
    std::vector<RegistroAlimento> findByPersona(std::int64_t personaId);
    bool remove(std::int64_t id);

    // Totales nutricionales de un día (Fase 2).
    struct TotalesDia {
        double calorias{ 0 };
        double carbohidratosG{ 0 };
        double proteinasG{ 0 };
        double grasasG{ 0 };
        double azucarG{ 0 };
        double grasasSaturadasG{ 0 };
        int registros{ 0 };
    };
    TotalesDia totalesPorDia(std::int64_t personaId, const std::string& fecha);

private:
    RegistroAlimento mapRow(class Statement& stmt) const;

    Database& db_;
};
