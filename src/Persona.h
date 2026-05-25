#pragma once

#include <cstdint>
#include <string>

// Persona registrada en el sistema (Fase 1 - requisito del proyecto).
struct Persona {
    std::int64_t id{ 0 };
    std::string nombreCompleto;
    int edad{ 0 };
    std::string genero;           // Masculino, Femenino, Otro
    std::string departamento;
    std::string municipio;
    double pesoKg{ 0.0 };
    double alturaCm{ 0.0 };
    std::string nivelActividad;   // Sedentario, Ligero, Moderado, Intenso, Muy intenso
    std::string ocupacion;
    std::string metaPeso{ "mantener" }; // bajar | mantener | subir
};
