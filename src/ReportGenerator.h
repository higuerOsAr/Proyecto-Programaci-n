#pragma once

#include "NutritionAnalyzer.h"
#include "Persona.h"
#include "RegistroAlimento.h"

#include <string>
#include <vector>

class ReportGenerator {
public:
    static std::string generarReporteCompleto(
        const Persona& persona,
        const std::string& fecha,
        const std::vector<RegistroAlimento>& alimentos,
        const NutritionAnalyzer::ImcResult& imc,
        const NutritionAnalyzer::AnalisisDia& analisis);

    static bool guardarEnArchivo(const std::string& ruta, const std::string& contenido);
};
