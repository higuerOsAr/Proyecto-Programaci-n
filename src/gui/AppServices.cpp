#include "AppServices.h"

AppServices& AppServices::instance() {
    static AppServices app;
    return app;
}

void AppServices::initialize(const char* dbPath) {
    db = std::make_unique<Database>(dbPath);
    personas = std::make_unique<PersonaRepository>(*db);
    alimentos = std::make_unique<AlimentoRepository>(*db);
    personas->createSchema();
    alimentos->createSchema();
}
