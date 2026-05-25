#include "PersonaRepository.h"

#include "Database.h"
#include "ReferenciasGuatemala.h"
#include "SqliteException.h"
#include "Statement.h"

PersonaRepository::PersonaRepository(Database& db) : db_(db) {}

void PersonaRepository::createSchema() {
    db_.exec(R"(
        CREATE TABLE IF NOT EXISTS personas (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            nombre_completo TEXT    NOT NULL,
            edad            INTEGER NOT NULL CHECK(edad > 0 AND edad < 130),
            genero          TEXT    NOT NULL,
            departamento      TEXT    NOT NULL,
            municipio       TEXT    NOT NULL,
            peso_kg         REAL    NOT NULL CHECK(peso_kg > 0),
            altura_cm       REAL    NOT NULL CHECK(altura_cm > 0),
            nivel_actividad TEXT    NOT NULL,
            ocupacion       TEXT    NOT NULL,
            meta_peso       TEXT    NOT NULL DEFAULT 'mantener',
            creado_en       TEXT    NOT NULL DEFAULT (datetime('now'))
        );
        CREATE INDEX IF NOT EXISTS idx_personas_depto ON personas(departamento, municipio);
    )");
}

void PersonaRepository::clearAll() {
    db_.exec("DELETE FROM personas;");
    db_.exec("DELETE FROM sqlite_sequence WHERE name IN ('personas', 'registros_alimenticios');");
}

std::int64_t PersonaRepository::insert(const Persona& p) {
    Persona copy = p;
    copy.metaPeso = RefGuatemala::normalizarMeta(p.metaPeso);

    auto stmt = db_.prepareCached(
        "insert_persona",
        "INSERT INTO personas (nombre_completo, edad, genero, departamento, municipio, "
        "peso_kg, altura_cm, nivel_actividad, ocupacion, meta_peso) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

    stmt.bind(1, copy.nombreCompleto);
    stmt.bind(2, copy.edad);
    stmt.bind(3, copy.genero);
    stmt.bind(4, copy.departamento);
    stmt.bind(5, copy.municipio);
    stmt.bind(6, copy.pesoKg);
    stmt.bind(7, copy.alturaCm);
    stmt.bind(8, copy.nivelActividad);
    stmt.bind(9, copy.ocupacion);
    stmt.bind(10, copy.metaPeso);
    stmt.run();

    return static_cast<std::int64_t>(db_.lastInsertRowId());
}

std::int64_t PersonaRepository::upsert(const Persona& p) {
    if (p.id > 0) {
        if (update(p)) {
            return p.id;
        }
    }
    return insert(p);
}

std::optional<Persona> PersonaRepository::findById(std::int64_t id) {
    auto stmt = db_.prepareCached(
        "find_persona_id",
        "SELECT id, nombre_completo, edad, genero, departamento, municipio, "
        "peso_kg, altura_cm, nivel_actividad, ocupacion, meta_peso "
        "FROM personas WHERE id = ?;");

    stmt.bind(1, id);
    if (!stmt.step()) {
        return std::nullopt;
    }
    return mapRow(stmt);
}

std::vector<Persona> PersonaRepository::findAll() {
    std::vector<Persona> result;
    auto stmt = db_.prepareCached(
        "find_all_personas",
        "SELECT id, nombre_completo, edad, genero, departamento, municipio, "
        "peso_kg, altura_cm, nivel_actividad, ocupacion, meta_peso "
        "FROM personas ORDER BY id;");

    while (stmt.step()) {
        result.push_back(mapRow(stmt));
    }
    return result;
}

bool PersonaRepository::update(const Persona& p) {
    Persona copy = p;
    copy.metaPeso = RefGuatemala::normalizarMeta(p.metaPeso);

    auto stmt = db_.prepareCached(
        "update_persona",
        "UPDATE personas SET nombre_completo=?, edad=?, genero=?, departamento=?, "
        "municipio=?, peso_kg=?, altura_cm=?, nivel_actividad=?, ocupacion=?, meta_peso=? "
        "WHERE id=?;");

    stmt.bind(1, copy.nombreCompleto);
    stmt.bind(2, copy.edad);
    stmt.bind(3, copy.genero);
    stmt.bind(4, copy.departamento);
    stmt.bind(5, copy.municipio);
    stmt.bind(6, copy.pesoKg);
    stmt.bind(7, copy.alturaCm);
    stmt.bind(8, copy.nivelActividad);
    stmt.bind(9, copy.ocupacion);
    stmt.bind(10, copy.metaPeso);
    stmt.bind(11, copy.id);
    stmt.run();
    return db_.changes() > 0;
}

bool PersonaRepository::remove(std::int64_t id) {
    auto stmt = db_.prepareCached("delete_persona", "DELETE FROM personas WHERE id = ?;");
    stmt.bind(1, id);
    stmt.run();
    return db_.changes() > 0;
}

Persona PersonaRepository::mapRow(Statement& stmt) const {
    Persona p;
    p.id = stmt.columnInt64(0);
    p.nombreCompleto = stmt.columnText(1);
    p.edad = stmt.columnInt(2);
    p.genero = stmt.columnText(3);
    p.departamento = stmt.columnText(4);
    p.municipio = stmt.columnText(5);
    p.pesoKg = stmt.columnDouble(6);
    p.alturaCm = stmt.columnDouble(7);
    p.nivelActividad = stmt.columnText(8);
    p.ocupacion = stmt.columnText(9);
    p.metaPeso = stmt.columnText(10);
    return p;
}
