#pragma once

class AlimentoRepository;
class PersonaRepository;

class ConsoleMenu {
public:
    ConsoleMenu(PersonaRepository& personas, AlimentoRepository& alimentos);

    void run();

private:
    void printMainMenu() const;
    void pause() const;

    // Fase 1
    void registrarPersona();
    void listarPersonas();
    void buscarPersona();
    void editarPersona();
    void eliminarPersona();
    void registrarAlimento();
    void verAlimentosDelDia();
    void verHistorialAlimentos();
    void eliminarAlimento();

    // Fase 2
    void mostrarImc();
    void mostrarRequerimientoCalorico();
    void analizarDia();
    void generarReportePantalla();
    void exportarReporteTxt();

    // Sistema
    void reiniciarBaseDatos();

    PersonaRepository& personas_;
    AlimentoRepository& alimentos_;
};
