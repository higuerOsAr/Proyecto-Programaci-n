#pragma once

#include "Persona.h"

#include <cstdint>
#include <optional>
#include <vector>

class Database;

class PersonaRepository {
public:
    explicit PersonaRepository(Database& db);

    void createSchema();
    void clearAll();
    std::int64_t insert(const Persona& p);
    std::int64_t upsert(const Persona& p);
    std::optional<Persona> findById(std::int64_t id);
    std::vector<Persona> findAll();
    bool update(const Persona& p);
    bool remove(std::int64_t id);

private:
    Persona mapRow(class Statement& stmt) const;

    Database& db_;
};
