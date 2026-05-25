#pragma once

#include <string>

// Valores de referencia alimentaria para Guatemala (MSPS / INCAP / Plato del Buen Comer).
namespace RefGuatemala {

inline constexpr const char* kFuenteOficial =
    "MSPS Guatemala - Ministerio de Salud Publica y Asistencia Social";

inline constexpr const char* kGuiaPlato =
    "Plato del Buen Comer (Guatemala): 50% cereales/tuberculos (maiz, tortilla, arroz), "
    "25% frutas y verduras, 12% leguminosas/huevos/carnes magras, 13% grasas saludables";

// Porcentaje de energia diaria segun distribucion recomendada en Guatemala.
inline constexpr double kPctCarbMin = 45.0;
inline constexpr double kPctCarbMax = 60.0;
inline constexpr double kPctProtMin = 10.0;
inline constexpr double kPctProtMax = 15.0;
inline constexpr double kPctGrasaMin = 25.0;
inline constexpr double kPctGrasaMax = 35.0;

// Azucares libres: OMS/MSPS recomiendan <10% energia; ~25 g/dia en dieta 2000 kcal.
inline constexpr double kAzucarMaxGramos = 25.0;

// Grasas saturadas: <10% energia total (~22 g en 2000 kcal).
inline constexpr double kGrasasSaturadasMaxGramos = 22.0;

// Ajuste calorico por meta de peso (kcal/dia) segun orientacion nutricional clinica MSPS.
inline constexpr double kAjusteBajarKcal = 400.0;
inline constexpr double kAjusteSubirKcal = 400.0;

std::string etiquetaMeta(const std::string& meta);
std::string normalizarMeta(const std::string& meta);

} // namespace RefGuatemala
