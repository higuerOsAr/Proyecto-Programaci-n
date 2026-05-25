#include "ConsoleMenu.h"

#include "AlimentoRepository.h"
#include "NutritionAnalyzer.h"
#include "Persona.h"
#include "PersonaRepository.h"
#include "RegistroAlimento.h"
#include "ReportGenerator.h"
#include "SqliteException.h"

#include <ctime>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace {

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

bool readYesNo(const std::string& prompt) {
    while (true) {
        const std::string a = readLine(prompt + " (s/n): ");
        if (a == "s" || a == "S" || a == "si") return true;
        if (a == "n" || a == "N" || a == "no") return false;
        std::cout << "  Responde s o n.\n";
    }
}

std::int64_t readInt64(const std::string& prompt) {
    while (true) {
        try {
            return std::stoll(readLine(prompt));
        } catch (...) {
            std::cout << "  Numero invalido.\n";
        }
    }
}

int readInt(const std::string& prompt) {
    return static_cast<int>(readInt64(prompt));
}

double readDouble(const std::string& prompt) {
    while (true) {
        try {
            return std::stod(readLine(prompt));
        } catch (...) {
            std::cout << "  Numero invalido.\n";
        }
    }
}

std::string fechaHoy() {
    std::time_t t = std::time(nullptr);
    std::tm lt{};
#ifdef _WIN32
    localtime_s(&lt, &t);
#else
    localtime_r(&t, &lt);
#endif
    char buf[16]{};
    std::snprintf(buf, sizeof buf, "%04d-%02d-%02d",
        lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
    return buf;
}

void printPersona(const Persona& p) {
    std::cout << "  [" << p.id << "] " << p.nombreCompleto
              << " | " << p.edad << " anios | " << p.genero
              << " | " << p.departamento << ", " << p.municipio << '\n'
              << "      Peso: " << p.pesoKg << " kg | Altura: " << p.alturaCm
              << " cm | Actividad: " << p.nivelActividad
              << " | Ocupacion: " << p.ocupacion << '\n';
}

void printAlimento(const RegistroAlimento& r) {
    std::cout << "  #" << r.id << " " << r.fecha << " | " << r.nombreProducto
              << " [" << r.cantidad << "] " << r.calorias << " kcal"
              << " (C:" << r.carbohidratosG << " P:" << r.proteinasG
              << " G:" << r.grasasG << " g)\n";
}

std::optional<Persona> pedirPersona(PersonaRepository& repo) {
    const auto id = readInt64("ID de la persona: ");
    const auto p = repo.findById(id);
    if (!p) {
        std::cout << "Persona no encontrada.\n";
    }
    return p;
}

} // namespace

ConsoleMenu::ConsoleMenu(PersonaRepository& personas, AlimentoRepository& alimentos)
    : personas_(personas)
    , alimentos_(alimentos) {}

void ConsoleMenu::pause() const {
    readLine("\n[Enter para continuar]");
}

void ConsoleMenu::printMainMenu() const {
    std::cout << "\n============================================================\n"
              << "  ANALISIS DE HABITOS ALIMENTICIOS - Proyecto Final 2026\n"
              << "  Programacion I | UMG Chimaltenango\n"
              << "============================================================\n"
              << " FASE 1 - Registro de datos\n"
              << "   1. Registrar persona\n"
              << "   2. Listar personas\n"
              << "   3. Buscar persona por ID\n"
              << "   4. Editar persona\n"
              << "   5. Eliminar persona\n"
              << "   6. Registrar alimento consumido\n"
              << "   7. Ver alimentos del dia\n"
              << "   8. Ver historial de alimentos\n"
              << "   9. Eliminar registro alimenticio\n"
              << " FASE 2 - Analisis y reportes (MSPS Guatemala)\n"
              << "  10. Calcular IMC\n"
              << "  11. Calcular requerimiento calorico diario\n"
              << "  12. Analisis nutricional del dia\n"
              << "  13. Generar reporte en pantalla\n"
              << "  14. Exportar reporte a archivo .txt\n"
              << " SISTEMA\n"
              << "  15. Reiniciar base de datos\n"
              << "   0. Salir\n"
              << "------------------------------------------------------------\n";
}

void ConsoleMenu::run() {
    while (true) {
        printMainMenu();
        const std::string op = readLine("Opcion: ");

        try {
            if (op == "0") { std::cout << "Hasta luego.\n"; break; }
            if (op == "1") { registrarPersona(); continue; }
            if (op == "2") { listarPersonas(); continue; }
            if (op == "3") { buscarPersona(); continue; }
            if (op == "4") { editarPersona(); continue; }
            if (op == "5") { eliminarPersona(); continue; }
            if (op == "6") { registrarAlimento(); continue; }
            if (op == "7") { verAlimentosDelDia(); continue; }
            if (op == "8") { verHistorialAlimentos(); continue; }
            if (op == "9") { eliminarAlimento(); continue; }
            if (op == "10") { mostrarImc(); continue; }
            if (op == "11") { mostrarRequerimientoCalorico(); continue; }
            if (op == "12") { analizarDia(); continue; }
            if (op == "13") { generarReportePantalla(); continue; }
            if (op == "14") { exportarReporteTxt(); continue; }
            if (op == "15") { reiniciarBaseDatos(); continue; }
            std::cout << "Opcion no valida.\n";
        } catch (const SqliteException& ex) {
            std::cerr << "Error SQLite (" << ex.code() << "): " << ex.what() << '\n';
            pause();
        }
    }
}

void ConsoleMenu::registrarPersona() {
    Persona p;
    p.nombreCompleto = readLine("Nombre completo: ");
    p.edad = readInt("Edad: ");
    p.genero = readLine("Genero (Masculino/Femenino/Otro): ");
    p.departamento = readLine("Departamento: ");
    p.municipio = readLine("Municipio: ");
    p.pesoKg = readDouble("Peso (kg): ");
    p.alturaCm = readDouble("Altura (cm): ");
    p.nivelActividad = readLine("Nivel actividad (Sedentario/Ligero/Moderado/Intenso/Muy intenso): ");
    p.ocupacion = readLine("Ocupacion/trabajo: ");
    std::cout << "Meta de peso: 1=Bajar  2=Mantener  3=Subir\n";
    const int metaOp = readInt("Opcion meta: ");
    if (metaOp == 1) p.metaPeso = "bajar";
    else if (metaOp == 3) p.metaPeso = "subir";
    else p.metaPeso = "mantener";

    const auto id = personas_.insert(p);
    std::cout << "Persona registrada con ID " << id << '\n';
    printPersona(*personas_.findById(id));
    pause();
}

void ConsoleMenu::listarPersonas() {
    const auto all = personas_.findAll();
    std::cout << "Total: " << all.size() << " persona(s)\n";
    for (const auto& p : all) {
        printPersona(p);
    }
    pause();
}

void ConsoleMenu::buscarPersona() {
    if (const auto p = pedirPersona(personas_)) {
        printPersona(*p);
    }
    pause();
}

void ConsoleMenu::eliminarPersona() {
    if (auto p = pedirPersona(personas_)) {
        if (readYesNo("Eliminar a " + p->nombreCompleto + "?")) {
            if (personas_.remove(p->id)) {
                std::cout << "Persona eliminada.\n";
            }
        }
    }
    pause();
}

void ConsoleMenu::editarPersona() {
    if (auto p = pedirPersona(personas_)) {
        std::cout << "Dejar vacio para conservar el valor actual.\n";
        const auto nombre = readLine("Nombre completo [" + p->nombreCompleto + "]: ");
        if (!nombre.empty()) p->nombreCompleto = nombre;

        const auto edadStr = readLine("Edad [" + std::to_string(p->edad) + "]: ");
        if (!edadStr.empty()) p->edad = std::stoi(edadStr);

        const auto pesoStr = readLine("Peso kg [" + std::to_string(p->pesoKg) + "]: ");
        if (!pesoStr.empty()) p->pesoKg = std::stod(pesoStr);

        const auto altStr = readLine("Altura cm [" + std::to_string(p->alturaCm) + "]: ");
        if (!altStr.empty()) p->alturaCm = std::stod(altStr);

        personas_.update(*p);
        std::cout << "Persona actualizada.\n";
        printPersona(*personas_.findById(p->id));
    }
    pause();
}

void ConsoleMenu::registrarAlimento() {
    if (auto p = pedirPersona(personas_)) {
        RegistroAlimento r;
        r.personaId = p->id;
        r.fecha = readLine("Fecha (YYYY-MM-DD) [hoy=" + fechaHoy() + "]: ");
        if (r.fecha.empty()) r.fecha = fechaHoy();

        r.nombreProducto = readLine("Nombre del producto: ");
        r.cantidad = readLine("Cantidad consumida (ej. 1 plato, 250 ml): ");
        r.calorias = readDouble("Calorias (kcal): ");
        r.carbohidratosG = readDouble("Carbohidratos (g): ");
        r.proteinasG = readDouble("Proteinas (g): ");
        r.grasasG = readDouble("Grasas (g): ");
        r.azucarG = readDouble("Azucar (g, 0 si no aplica): ");
        r.grasasSaturadasG = readDouble("Grasas saturadas (g, 0 si no aplica): ");

        const auto id = alimentos_.insert(r);
        std::cout << "Alimento registrado con ID " << id << '\n';
        printAlimento(*alimentos_.findById(id));
    }
    pause();
}

void ConsoleMenu::verAlimentosDelDia() {
    if (auto p = pedirPersona(personas_)) {
        std::string fecha = readLine("Fecha [" + fechaHoy() + "]: ");
        if (fecha.empty()) fecha = fechaHoy();

        const auto lista = alimentos_.findByPersonaAndFecha(p->id, fecha);
        std::cout << "Alimentos de " << p->nombreCompleto << " el " << fecha << ":\n";
        for (const auto& r : lista) {
            printAlimento(r);
        }
        const auto t = alimentos_.totalesPorDia(p->id, fecha);
        std::cout << "Totales: " << t.calorias << " kcal | C:" << t.carbohidratosG
                  << " P:" << t.proteinasG << " G:" << t.grasasG << " g\n";
    }
    pause();
}

void ConsoleMenu::verHistorialAlimentos() {
    if (auto p = pedirPersona(personas_)) {
        const auto lista = alimentos_.findByPersona(p->id);
        for (const auto& r : lista) {
            printAlimento(r);
        }
        std::cout << "Total registros: " << lista.size() << '\n';
    }
    pause();
}

void ConsoleMenu::eliminarAlimento() {
    const auto id = readInt64("ID del registro alimenticio: ");
    if (alimentos_.remove(id)) {
        std::cout << "Registro eliminado.\n";
    } else {
        std::cout << "No encontrado.\n";
    }
    pause();
}

void ConsoleMenu::mostrarImc() {
    if (auto p = pedirPersona(personas_)) {
        NutritionAnalyzer analyzer;
        const auto imc = analyzer.calcularImc(*p);
        std::cout << "IMC: " << imc.valor << " -> " << imc.clasificacion << '\n'
                  << imc.recomendacion << '\n';
    }
    pause();
}

void ConsoleMenu::mostrarRequerimientoCalorico() {
    if (auto p = pedirPersona(personas_)) {
        NutritionAnalyzer analyzer;
        const auto c = analyzer.calcularRequerimientoCalorico(*p);
        std::cout << c.formulaUsada << '\n'
                  << "TMB: " << c.tmb << " kcal\n"
                  << "Factor actividad: " << c.factorActividad << '\n'
                  << "Requerimiento diario: " << c.requerimientoDiario << " kcal\n";
    }
    pause();
}

void ConsoleMenu::analizarDia() {
    if (auto p = pedirPersona(personas_)) {
        std::string fecha = readLine("Fecha [" + fechaHoy() + "]: ");
        if (fecha.empty()) fecha = fechaHoy();

        NutritionAnalyzer analyzer;
        const auto totales = alimentos_.totalesPorDia(p->id, fecha);
        const auto analisis = analyzer.analizarDia(*p, totales);

        std::cout << "\n--- Analisis del " << fecha << " ---\n";
        std::cout << "Calorias consumidas: " << analisis.consumido.calorias << " kcal\n";
        std::cout << "Calorias recomendadas: " << analisis.calorias.requerimientoDiario << " kcal\n";
        std::cout << "Diferencia: " << analisis.diferenciaCalorias << " kcal\n\n";

        std::cout << "Alertas:\n";
        for (const auto& a : analisis.alertas) {
            std::cout << "  ! " << a << '\n';
        }
        std::cout << "Recomendaciones:\n";
        for (const auto& r : analisis.recomendaciones) {
            std::cout << "  > " << r << '\n';
        }
    }
    pause();
}

void ConsoleMenu::generarReportePantalla() {
    if (auto p = pedirPersona(personas_)) {
        std::string fecha = readLine("Fecha [" + fechaHoy() + "]: ");
        if (fecha.empty()) fecha = fechaHoy();

        NutritionAnalyzer analyzer;
        const auto imc = analyzer.calcularImc(*p);
        const auto totales = alimentos_.totalesPorDia(p->id, fecha);
        const auto analisis = analyzer.analizarDia(*p, totales);
        const auto lista = alimentos_.findByPersonaAndFecha(p->id, fecha);

        const auto reporte = ReportGenerator::generarReporteCompleto(
            *p, fecha, lista, imc, analisis);
        std::cout << reporte;
    }
    pause();
}

void ConsoleMenu::exportarReporteTxt() {
    if (auto p = pedirPersona(personas_)) {
        std::string fecha = readLine("Fecha [" + fechaHoy() + "]: ");
        if (fecha.empty()) fecha = fechaHoy();

        NutritionAnalyzer analyzer;
        const auto imc = analyzer.calcularImc(*p);
        const auto totales = alimentos_.totalesPorDia(p->id, fecha);
        const auto analisis = analyzer.analizarDia(*p, totales);
        const auto lista = alimentos_.findByPersonaAndFecha(p->id, fecha);

        const auto contenido = ReportGenerator::generarReporteCompleto(
            *p, fecha, lista, imc, analisis);

        const std::string ruta = "reporte_persona" + std::to_string(p->id) + "_" + fecha + ".txt";
        if (ReportGenerator::guardarEnArchivo(ruta, contenido)) {
            std::cout << "Reporte guardado en: " << ruta << '\n';
        } else {
            std::cout << "No se pudo guardar el archivo.\n";
        }
    }
    pause();
}

void ConsoleMenu::reiniciarBaseDatos() {
    if (readYesNo("Seguro? Se borraran personas y alimentos")) {
        personas_.clearAll();
        std::cout << "Base de datos reiniciada.\n";
    }
    pause();
}
