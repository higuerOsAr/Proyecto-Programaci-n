#pragma once

#include "AlimentoRepository.h"
#include "Persona.h"

#include <string>
#include <vector>

// Análisis nutricional (Fase 2 del proyecto).
class NutritionAnalyzer {
public:
    struct ImcResult {
        double valor{ 0 };
        std::string clasificacion;
        std::string recomendacion;
    };

    struct CaloriasResult {
        double tmb{ 0 };           // Tasa metabólica basal (Mifflin-St Jeor)
        double factorActividad{ 0 };
        double requerimientoDiario{ 0 };
        std::string formulaUsada;
    };

    struct AnalisisDia {
        AlimentoRepository::TotalesDia consumido;
        CaloriasResult calorias;
        double diferenciaCalorias{ 0 }; // consumido - recomendado
        std::vector<std::string> alertas;
        std::vector<std::string> recomendaciones;
    };

    ImcResult calcularImc(const Persona& p) const;
    CaloriasResult calcularRequerimientoCalorico(const Persona& p) const;
    AnalisisDia analizarDia(const Persona& p, const AlimentoRepository::TotalesDia& totales) const;
    std::vector<std::string> sugerirAlternativas(const AnalisisDia& analisis) const;

private:
    double factorActividad(const std::string& nivel) const;
    void analizarMacronutrientes(const Persona& p, const AlimentoRepository::TotalesDia& t,
        double caloriasRecomendadas, AnalisisDia& out) const;
};
