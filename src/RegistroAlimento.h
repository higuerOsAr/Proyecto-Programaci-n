#pragma once

#include <cstdint>
#include <string>

// Alimento consumido en un día (Fase 1).
struct RegistroAlimento {
    std::int64_t id{ 0 };
    std::int64_t personaId{ 0 };
    std::string fecha;            // YYYY-MM-DD
    std::string nombreProducto;
    std::string cantidad;         // ej. "1 taza", "200 g"
    double calorias{ 0.0 };
    double carbohidratosG{ 0.0 };
    double proteinasG{ 0.0 };
    double grasasG{ 0.0 };
    double azucarG{ 0.0 };
    double grasasSaturadasG{ 0.0 };
};
