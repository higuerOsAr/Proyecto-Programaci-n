#include "AlimentoRepository.h"

#include "Database.h"
#include "Statement.h"

AlimentoRepository::AlimentoRepository(Database& db) : db_(db) {}

void AlimentoRepository::createSchema() {
    db_.exec(R"(
        CREATE TABLE IF NOT EXISTS registros_alimenticios (
            id                INTEGER PRIMARY KEY AUTOINCREMENT,
            persona_id        INTEGER NOT NULL,
            fecha             TEXT    NOT NULL DEFAULT (date('now')),
            nombre_producto   TEXT    NOT NULL,
            cantidad          TEXT    NOT NULL,
            calorias          REAL    NOT NULL CHECK(calorias >= 0),
            carbohidratos_g   REAL    NOT NULL CHECK(carbohidratos_g >= 0),
            proteinas_g       REAL    NOT NULL CHECK(proteinas_g >= 0),
            grasas_g          REAL    NOT NULL CHECK(grasas_g >= 0),
            azucar_g          REAL    NOT NULL DEFAULT 0 CHECK(azucar_g >= 0),
            grasas_saturadas_g REAL   NOT NULL DEFAULT 0 CHECK(grasas_saturadas_g >= 0),
            FOREIGN KEY (persona_id) REFERENCES personas(id) ON DELETE CASCADE
        );
        CREATE INDEX IF NOT EXISTS idx_alimentos_persona_fecha
            ON registros_alimenticios(persona_id, fecha);
    )");
}

std::int64_t AlimentoRepository::insert(const RegistroAlimento& r) {
    auto stmt = db_.prepareCached(
        "insert_alimento",
        "INSERT INTO registros_alimenticios "
        "(persona_id, fecha, nombre_producto, cantidad, calorias, carbohidratos_g, "
        "proteinas_g, grasas_g, azucar_g, grasas_saturadas_g) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

    stmt.bind(1, r.personaId);
    stmt.bind(2, r.fecha);
    stmt.bind(3, r.nombreProducto);
    stmt.bind(4, r.cantidad);
    stmt.bind(5, r.calorias);
    stmt.bind(6, r.carbohidratosG);
    stmt.bind(7, r.proteinasG);
    stmt.bind(8, r.grasasG);
    stmt.bind(9, r.azucarG);
    stmt.bind(10, r.grasasSaturadasG);
    stmt.run();

    return static_cast<std::int64_t>(db_.lastInsertRowId());
}

std::optional<RegistroAlimento> AlimentoRepository::findById(std::int64_t id) {
    auto stmt = db_.prepareCached(
        "find_alimento_id",
        "SELECT id, persona_id, fecha, nombre_producto, cantidad, calorias, "
        "carbohidratos_g, proteinas_g, grasas_g, azucar_g, grasas_saturadas_g "
        "FROM registros_alimenticios WHERE id = ?;");

    stmt.bind(1, id);
    if (!stmt.step()) {
        return std::nullopt;
    }
    return mapRow(stmt);
}

std::vector<RegistroAlimento> AlimentoRepository::findByPersonaAndFecha(
    std::int64_t personaId, const std::string& fecha) {
    auto stmt = db_.prepareCached(
        "find_alimentos_dia",
        "SELECT id, persona_id, fecha, nombre_producto, cantidad, calorias, "
        "carbohidratos_g, proteinas_g, grasas_g, azucar_g, grasas_saturadas_g "
        "FROM registros_alimenticios WHERE persona_id = ? AND fecha = ? ORDER BY id;");

    stmt.bind(1, personaId);
    stmt.bind(2, fecha);

    std::vector<RegistroAlimento> result;
    while (stmt.step()) {
        result.push_back(mapRow(stmt));
    }
    return result;
}

std::vector<RegistroAlimento> AlimentoRepository::findByPersona(std::int64_t personaId) {
    auto stmt = db_.prepareCached(
        "find_alimentos_persona",
        "SELECT id, persona_id, fecha, nombre_producto, cantidad, calorias, "
        "carbohidratos_g, proteinas_g, grasas_g, azucar_g, grasas_saturadas_g "
        "FROM registros_alimenticios WHERE persona_id = ? ORDER BY fecha DESC, id;");

    stmt.bind(1, personaId);

    std::vector<RegistroAlimento> result;
    while (stmt.step()) {
        result.push_back(mapRow(stmt));
    }
    return result;
}

bool AlimentoRepository::remove(std::int64_t id) {
    auto stmt = db_.prepareCached("delete_alimento", "DELETE FROM registros_alimenticios WHERE id = ?;");
    stmt.bind(1, id);
    stmt.run();
    return db_.changes() > 0;
}

AlimentoRepository::TotalesDia AlimentoRepository::totalesPorDia(
    std::int64_t personaId, const std::string& fecha) {
    auto stmt = db_.prepareCached(
        "totales_dia",
        "SELECT COUNT(*), COALESCE(SUM(calorias),0), COALESCE(SUM(carbohidratos_g),0), "
        "COALESCE(SUM(proteinas_g),0), COALESCE(SUM(grasas_g),0), "
        "COALESCE(SUM(azucar_g),0), COALESCE(SUM(grasas_saturadas_g),0) "
        "FROM registros_alimenticios WHERE persona_id = ? AND fecha = ?;");

    stmt.bind(1, personaId);
    stmt.bind(2, fecha);
    stmt.step();

    TotalesDia t;
    t.registros = stmt.columnInt(0);
    t.calorias = stmt.columnDouble(1);
    t.carbohidratosG = stmt.columnDouble(2);
    t.proteinasG = stmt.columnDouble(3);
    t.grasasG = stmt.columnDouble(4);
    t.azucarG = stmt.columnDouble(5);
    t.grasasSaturadasG = stmt.columnDouble(6);
    return t;
}

RegistroAlimento AlimentoRepository::mapRow(Statement& stmt) const {
    RegistroAlimento r;
    r.id = stmt.columnInt64(0);
    r.personaId = stmt.columnInt64(1);
    r.fecha = stmt.columnText(2);
    r.nombreProducto = stmt.columnText(3);
    r.cantidad = stmt.columnText(4);
    r.calorias = stmt.columnDouble(5);
    r.carbohidratosG = stmt.columnDouble(6);
    r.proteinasG = stmt.columnDouble(7);
    r.grasasG = stmt.columnDouble(8);
    r.azucarG = stmt.columnDouble(9);
    r.grasasSaturadasG = stmt.columnDouble(10);
    return r;
}
