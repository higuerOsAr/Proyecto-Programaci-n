#include "ReportGenerator.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

std::string linea(char c = '=', int n = 60) {
    return std::string(n, c) + "\n";
}

} // namespace

std::string ReportGenerator::generarReporteCompleto(
    const Persona& persona,
    const std::string& fecha,
    const std::vector<RegistroAlimento>& alimentos,
    const NutritionAnalyzer::ImcResult& imc,
    const NutritionAnalyzer::AnalisisDia& analisis) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1);

    out << linea();
    out << "  REPORTE NUTRICIONAL - GUATEMALA\n";
    out << "  Proyecto Programacion I - UMG Chimaltenango\n";
    out << "  Referencia: MSPS Guatemala - Plato del Buen Comer\n";
    out << linea();
    out << "Fecha del analisis: " << fecha << "\n\n";

    out << "--- DATOS DE LA PERSONA ---\n";
    out << "ID: " << persona.id << "\n";
    out << "Nombre: " << persona.nombreCompleto << "\n";
    out << "Edad: " << persona.edad << " | Genero: " << persona.genero << "\n";
    out << "Ubicacion: " << persona.departamento << ", " << persona.municipio << "\n";
    const double pesoLb = persona.pesoKg * 2.2046226218;
    out << "Peso: " << pesoLb << " lb (" << persona.pesoKg << " kg) | Altura: " << persona.alturaCm << " cm\n";
    out << "Actividad: " << persona.nivelActividad << " | Ocupacion: " << persona.ocupacion << "\n";
    out << "Meta: " << persona.metaPeso << " (bajar / mantener / subir)\n\n";

    out << "--- INDICE DE MASA CORPORAL (IMC) ---\n";
    out << "IMC: " << imc.valor << " -> " << imc.clasificacion << "\n";
    out << imc.recomendacion << "\n\n";

    out << "--- REQUERIMIENTO CALORICO ---\n";
    out << "Formula: " << analisis.calorias.formulaUsada << "\n";
    out << "TMB: " << analisis.calorias.tmb << " kcal\n";
    out << "Factor actividad: " << analisis.calorias.factorActividad << "\n";
    out << "Requerimiento diario estimado: " << analisis.calorias.requerimientoDiario << " kcal\n\n";

    out << "--- ALIMENTOS REGISTRADOS (" << alimentos.size() << ") ---\n";
    for (const auto& a : alimentos) {
        out << "  * " << a.nombreProducto << " [" << a.cantidad << "] "
            << a.calorias << " kcal | C:" << a.carbohidratosG
            << "g P:" << a.proteinasG << "g G:" << a.grasasG << "g\n";
    }
    out << "\n";

    out << "--- TOTALES DEL DIA ---\n";
    out << "Calorias: " << analisis.consumido.calorias << " kcal\n";
    out << "Carbohidratos: " << analisis.consumido.carbohidratosG << " g\n";
    out << "Proteinas: " << analisis.consumido.proteinasG << " g\n";
    out << "Grasas: " << analisis.consumido.grasasG << " g\n";
    out << "Azucar: " << analisis.consumido.azucarG << " g\n";
    out << "Grasas saturadas: " << analisis.consumido.grasasSaturadasG << " g\n";
    out << "Diferencia (consumido - recomendado): " << analisis.diferenciaCalorias << " kcal\n\n";

    out << "--- ALERTAS ---\n";
    if (analisis.alertas.empty()) {
        out << "  Sin alertas criticas para este dia.\n";
    } else {
        for (const auto& al : analisis.alertas) {
            out << "  ! " << al << "\n";
        }
    }
    out << "\n--- RECOMENDACIONES ---\n";
    for (const auto& rec : analisis.recomendaciones) {
        out << "  > " << rec << "\n";
    }

    out << "\n" << linea();
    out << "Nota: Valores de referencia basados en MSPS Guatemala e INCAP.\n";
    out << "Este reporte es orientativo y no sustituye consulta medica.\n";
    out << linea();

    return out.str();
}

bool ReportGenerator::guardarEnArchivo(const std::string& ruta, const std::string& contenido) {
    std::ofstream file(ruta, std::ios::out | std::ios::trunc);
    if (!file) {
        return false;
    }
    file << contenido;
    return file.good();
}
