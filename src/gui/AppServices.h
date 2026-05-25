#pragma once

#include "AlimentoRepository.h"
#include "Database.h"
#include "NutritionAnalyzer.h"
#include "PersonaRepository.h"

#include <memory>

struct AppServices {
    std::unique_ptr<Database> db;
    std::unique_ptr<PersonaRepository> personas;
    std::unique_ptr<AlimentoRepository> alimentos;
    NutritionAnalyzer analyzer;

    static AppServices& instance();

    void initialize(const char* dbPath = "nutricion.db");

private:
    AppServices() = default;
};
