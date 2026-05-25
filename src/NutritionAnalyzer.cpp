#include "NutritionAnalyzer.h"

#include "ReferenciasGuatemala.h"

#include <algorithm>
#include <cmath>
#include <cctype>

namespace {

std::string toLower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

bool generoEsFemenino(const std::string& genero) {
    const auto g = toLower(genero);
    return g == "femenino" || g == "f" || g == "mujer";
}

} // namespace

NutritionAnalyzer::ImcResult NutritionAnalyzer::calcularImc(const Persona& p) const {
    const double alturaM = p.alturaCm / 100.0;
    ImcResult r;
    r.valor = p.pesoKg / (alturaM * alturaM);

    // Clasificacion IMC usada por MSPS Guatemala (basada en criterios OMS para poblacion adulta).
    if (r.valor < 18.5) {
        r.clasificacion = "Bajo peso (criterio MSPS/OMS)";
        r.recomendacion =
            "Segun " + std::string(RefGuatemala::kFuenteOficial) +
            ": refuerce consumo de tortilla de maiz, frijol, huevo y verduras de temporada en Guatemala.";
    } else if (r.valor < 25.0) {
        r.clasificacion = "Peso saludable (criterio MSPS/OMS)";
        r.recomendacion =
            "Mantenga el Plato del Buen Comer guatemalteco: maiz, frijol, frutas tropicales y agua potable.";
    } else if (r.valor < 30.0) {
        r.clasificacion = "Sobrepeso (criterio MSPS/OMS)";
        r.recomendacion =
            "MSPS Guatemala: reduzca refrescos, pan dulce y frituras; priorice sopas de verdura, pollo y leguminosas.";
    } else {
        r.clasificacion = "Obesidad (criterio MSPS/OMS)";
        r.recomendacion =
            "Acuda al servicio de salud mas cercano en su municipio guatemalteco para plan alimentario supervisado.";
    }
    return r;
}

double NutritionAnalyzer::factorActividad(const std::string& nivel) const {
    // Factores de actividad aplicados en valoraciones nutricionales de adultos (MSPS/INCAP Guatemala).
    const auto n = toLower(nivel);
    if (n.find("sedent") != std::string::npos) return 1.2;
    if (n.find("ligero") != std::string::npos || n.find("leve") != std::string::npos) return 1.375;
    if (n.find("moder") != std::string::npos) return 1.55;
    if (n.find("muy") != std::string::npos) return 1.9;
    if (n.find("inten") != std::string::npos) return 1.725;
    return 1.2;
}

NutritionAnalyzer::CaloriasResult NutritionAnalyzer::calcularRequerimientoCalorico(const Persona& p) const {
    CaloriasResult r;
    r.formulaUsada =
        "Requerimiento energetico adulto (Mifflin-St Jeor) ajustado segun meta de peso - referencia MSPS Guatemala";

    if (generoEsFemenino(p.genero)) {
        r.tmb = 10.0 * p.pesoKg + 6.25 * p.alturaCm - 5.0 * p.edad - 161.0;
    } else {
        r.tmb = 10.0 * p.pesoKg + 6.25 * p.alturaCm - 5.0 * p.edad + 5.0;
    }

    r.factorActividad = factorActividad(p.nivelActividad);
    r.requerimientoDiario = r.tmb * r.factorActividad;

    const auto meta = RefGuatemala::normalizarMeta(p.metaPeso);
    if (meta == "bajar") {
        r.requerimientoDiario -= RefGuatemala::kAjusteBajarKcal;
    } else if (meta == "subir") {
        r.requerimientoDiario += RefGuatemala::kAjusteSubirKcal;
    }

    return r;
}

void NutritionAnalyzer::analizarMacronutrientes(
    const Persona& p,
    const AlimentoRepository::TotalesDia& t,
    double caloriasRecomendadas,
    AnalisisDia& out) const {
    out.recomendaciones.insert(out.recomendaciones.begin(),
        std::string("Referencia nacional: ") + RefGuatemala::kFuenteOficial);
    out.recomendaciones.insert(out.recomendaciones.begin() + 1, RefGuatemala::kGuiaPlato);

    if (t.registros == 0) {
        out.alertas.push_back("No hay alimentos registrados para este dia.");
        return;
    }

    const double cal = (std::max)(t.calorias, 1.0);
    const double pctCarb = (t.carbohidratosG * 4.0 / cal) * 100.0;
    const double pctProt = (t.proteinasG * 4.0 / cal) * 100.0;
    const double pctGrasa = (t.grasasG * 9.0 / cal) * 100.0;

    if (pctCarb > RefGuatemala::kPctCarbMax) {
        out.alertas.push_back(
            "Exceso de carbohidratos vs Plato del Buen Comer Guatemala (~" +
            std::to_string(static_cast<int>(pctCarb)) + "%; max ~" +
            std::to_string(static_cast<int>(RefGuatemala::kPctCarbMax)) + "%).");
    }
    if (pctCarb < RefGuatemala::kPctCarbMin) {
        out.alertas.push_back(
            "Deficit de carbohidratos vs dieta guatemalteca (~" +
            std::to_string(static_cast<int>(pctCarb)) + "%; incluya maiz, tortilla o arroz).");
    }
    if (pctProt < RefGuatemala::kPctProtMin) {
        out.alertas.push_back(
            "Deficit de proteinas (~" + std::to_string(static_cast<int>(pctProt)) +
            "%; MSPS sugiere frijol, huevo, pollo o pescado).");
    }
    if (pctGrasa > RefGuatemala::kPctGrasaMax) {
        out.alertas.push_back(
            "Exceso de grasas (~" + std::to_string(static_cast<int>(pctGrasa)) +
            "%; limite referencia Guatemala ~" +
            std::to_string(static_cast<int>(RefGuatemala::kPctGrasaMax)) + "%).");
    }

    if (t.azucarG > RefGuatemala::kAzucarMaxGramos) {
        out.alertas.push_back(
            "Alto consumo de azucar (" + std::to_string(static_cast<int>(t.azucarG)) +
            " g). MSPS/OMS Guatemala: limitar azucares libres (<" +
            std::to_string(static_cast<int>(RefGuatemala::kAzucarMaxGramos)) + " g/dia).");
    }
    if (t.grasasSaturadasG > RefGuatemala::kGrasasSaturadasMaxGramos) {
        out.alertas.push_back(
            "Alto consumo de grasas saturadas (" +
            std::to_string(static_cast<int>(t.grasasSaturadasG)) +
            " g). Reduzca chicharron, manteca y frituras tipicas.");
    }

    const auto diff = t.calorias - caloriasRecomendadas;
    if (diff > 300.0) {
        out.alertas.push_back(
            "Calorias por encima del requerimiento estimado para su perfil en Guatemala.");
    } else if (diff < -300.0) {
        out.alertas.push_back(
            "Calorias por debajo del requerimiento estimado; riesgo de deficit energetico.");
    }

    const auto meta = RefGuatemala::normalizarMeta(p.metaPeso);
    if (meta == "bajar") {
        out.recomendaciones.push_back(
            "Meta BAJAR (MSPS GT): sustituya refrescos por agua purificada; menos tamal frito y mas ensalada.");
        out.recomendaciones.push_back(
            "Platos sugeridos: pepian de pollo con verdura, frijoles sin manteca, tortilla de maiz moderada.");
    } else if (meta == "subir") {
        out.recomendaciones.push_back(
            "Meta SUBIR (MSPS GT): agregue atol de elote, platano, aguacate y huevo; aumente porciones de frijol.");
        out.recomendaciones.push_back(
            "Incluya 3 comidas principales + refrigerio con fruta de temporada (mango, papaya, banana).");
    } else {
        out.recomendaciones.push_back(
            "Meta MANTENER: dieta tradicional balanceada (tortilla, frijol, verdura y proteina magra).");
    }

    out.recomendaciones.push_back(
        "Ubicacion registrada: " + p.departamento + ", " + p.municipio +
        " - considere productos locales de su region.");
}

NutritionAnalyzer::AnalisisDia NutritionAnalyzer::analizarDia(
    const Persona& p, const AlimentoRepository::TotalesDia& totales) const {
    AnalisisDia a;
    a.consumido = totales;
    a.calorias = calcularRequerimientoCalorico(p);
    a.diferenciaCalorias = totales.calorias - a.calorias.requerimientoDiario;
    analizarMacronutrientes(p, totales, a.calorias.requerimientoDiario, a);
    return a;
}

std::vector<std::string> NutritionAnalyzer::sugerirAlternativas(const AnalisisDia& analisis) const {
    std::vector<std::string> alt;

    for (const auto& alerta : analisis.alertas) {
        const auto a = toLower(alerta);
        if (a.find("azucar") != std::string::npos) {
            alt.push_back("Guatemala: cambie refresco por agua, horchata sin azucar o te de manzanilla.");
            alt.push_back("Postre: fruta local (jocote, mango) en lugar de dulces procesados.");
        }
        if (a.find("grasas saturadas") != std::string::npos || a.find("grasas") != std::string::npos) {
            alt.push_back("Evite chicharron y empanadas fritas; prefiera caldo de pollo y pescado del lago o costa.");
        }
        if (a.find("carbohidrato") != std::string::npos && a.find("exceso") != std::string::npos) {
            alt.push_back("Reduzca pan dulce/shuco; mantenga 2-3 tortillas de maiz por comida.");
        }
        if (a.find("carbohidrato") != std::string::npos && a.find("deficit") != std::string::npos) {
            alt.push_back("Agregue tortilla de maiz, camote o arroz como base energetica guatemalteca.");
        }
        if (a.find("prote") != std::string::npos && a.find("deficit") != std::string::npos) {
            alt.push_back("Incluya frijol negro, huevo, queso fresco o pollo guisado (fontes comunes en GT).");
        }
        if (a.find("encima") != std::string::npos) {
            alt.push_back("Reduzca porcion de tamal y botanas; aumente ensalada de rabano y tomate.");
        }
        if (a.find("debajo") != std::string::npos) {
            alt.push_back("Agregue atol, platano macho y granos (avena, maiz) en desayuno.");
        }
    }

    if (alt.empty()) {
        alt.push_back("Dieta acorde al Plato del Buen Comer de Guatemala: maiz, frijol, verdura y fruta de temporada.");
        alt.push_back("Fuente: " + std::string(RefGuatemala::kFuenteOficial) + ".");
    }

    return alt;
}
