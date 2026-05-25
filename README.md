# Análisis de Hábitos Alimenticios — Proyecto Final 2026

**Universidad Mariano Gálvez** — Programación I, Sección B  
**Centro Universitario de Chimaltenango**

Software en **C++** con **Visual Studio** y **SQLite** para registrar hábitos alimenticios, analizar nutrición y generar recomendaciones de salud.

## Requisitos del PDF implementados

| Fase | Requisito | Estado |
|------|-----------|--------|
| **1** | Interfaz consola | ✅ (`--console`) |
| **1/3** | Interfaz gráfica WinAPI | ✅ |
| **1** | Registro de persona (nombre, edad, género, depto, municipio, peso, altura, actividad, ocupación) | ✅ |
| **1** | Registro de alimentos (producto, cantidad, calorías, carbohidratos, proteínas, grasas) | ✅ |
| **1** | Almacenamiento SQL | ✅ |
| **2** | Cálculo IMC | ✅ |
| **2** | Requerimiento calórico (Mifflin-St Jeor) | ✅ |
| **2** | Comparación calorías ingeridas vs recomendadas | ✅ |
| **2** | Alertas macronutrientes, azúcar, grasas saturadas | ✅ |
| **2** | Reporte en pantalla y archivo `.txt` | ✅ |
| **3** | Recomendaciones según meta (bajar/mantener/subir) + MSPS Guatemala | ✅ |
| **3** | Gráficos (barras GDI / WinAPI) | ✅ |
| **3** | Alternativas alimentarias personalizadas | ✅ |

## Cómo abrir y compilar

1. Doble clic en **`DatabaseApp.sln`**
2. Configuración: **Release | x64**
3. Compilar (Ctrl+Mayús+B) → Ejecutar (Ctrl+F5) → **ventana gráfica**
4. **Meta de peso:** lista con Bajar / Mantener / Subir
5. **Eliminar persona** y formulario que se limpia al guardar
6. Análisis con referencias **MSPS Guatemala** (Plato del Buen Comer)
7. Consola opcional: `DatabaseApp.exe --console`

Base de datos: `nutricion.db` (junto al `.exe` en `bin\Release\`)

## Estructura del código

```
src/
  Persona.h / PersonaRepository.*     — Fase 1: usuarios
  RegistroAlimento.h / AlimentoRepository.* — Fase 1: alimentos
  NutritionAnalyzer.*                 — Fase 2: IMC, calorías, alertas
  ReportGenerator.*                   — Fase 2: reportes TXT
  gui/MainWindow.*, ChartWindow.*     — Interfaz gráfica + gráficos
  ConsoleMenu.*                       — Interfaz consola (--console)
  Database.* / Statement.* / Transaction.*
docs/
  MODELO_ER.md                        — Entregable Fase 1
  INFORME_PRUEBAS.md                  — Entregable Fase 1
```

## Entregables del curso

| Documento | Ubicación |
|-----------|-----------|
| Modelo entidad-relación | `docs/MODELO_ER.md` |
| Informe de pruebas | `docs/INFORME_PRUEBAS.md` |
| Código fuente | carpeta `src/` |
| Video | Lo graba el equipo (ver roles en `INFORME_PRUEBAS.md`) |

## Roles del equipo (4 personas)

- **Backend:** `Database`, repositorios, SQLite
- **Frontend:** `ConsoleMenu`, entrada/salida
- **Analista:** `NutritionAnalyzer`, `ReportGenerator`
- **Cuarto integrante:** documentación, pruebas, video, integración

## Interfaz gráfica (Fase 3)

- Ventana principal WinAPI: registro persona, alimentos, análisis, reporte TXT
- Ventana de **gráficos** (barras): calorías consumidas vs recomendadas, macronutrientes
- Rol **Frontend** en el video: demostrar la ventana y botones
