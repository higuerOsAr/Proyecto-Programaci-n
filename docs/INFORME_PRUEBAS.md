# Informe de pruebas — Almacenamiento y recuperación de datos

**Proyecto:** Análisis de hábitos alimenticios  
**Fase evaluada:** 1 y 2 (almacenamiento + análisis)

## Entorno

- Visual Studio 2022 / 2026, C++20, x64 Release
- SQLite 3 (amalgamation) — archivo `nutricion.db`
- Interfaz gráfica WinAPI (predeterminada) + consola (`--console`)

## Casos de prueba

| # | Prueba | Pasos | Resultado esperado | Estado |
|---|--------|-------|-------------------|--------|
| 1 | Crear persona | Menú 1, ingresar datos completos | ID generado, registro en BD | OK |
| 2 | Listar personas | Menú 2 | Muestra todos los registros | OK |
| 3 | Buscar por ID | Menú 3 | Datos correctos de la persona | OK |
| 4 | Registrar alimento | Menú 5, asociar a persona | Registro con fecha y macros | OK |
| 5 | Recuperar alimentos del día | Menú 6 | Lista y totales del día | OK |
| 6 | Historial | Menú 7 | Todos los alimentos de la persona | OK |
| 7 | Eliminar alimento | Menú 8 | Registro eliminado | OK |
| 8 | Calcular IMC | Menú 9 | Valor y clasificación OMS | OK |
| 9 | Requerimiento calórico | Menú 10 | TMB y kcal diarias (Mifflin-St Jeor) | OK |
| 10 | Análisis del día | Menú 11 | Alertas y recomendaciones | OK |
| 11 | Reporte en pantalla | Menú 12 | Reporte formateado | OK |
| 12 | Exportar TXT | Menú 13 | Archivo `.txt` generado | OK |
| 13 | Datos de ejemplo | Menú 14 | Persona + 5 alimentos | OK |
| 14 | Reinicio BD | Menú 15 | Tablas vacías | OK |
| 15 | Integridad referencial | Eliminar persona con alimentos | CASCADE elimina alimentos | OK |
| 16 | GUI registrar persona | Ventana principal, Guardar persona | Aparece en listbox | OK |
| 17 | GUI agregar alimento | Seleccionar persona + Agregar | Listado del día actualizado | OK |
| 18 | GUI análisis | Botón Análisis del día | Panel de alertas/recomendaciones | OK |
| 19 | GUI gráficos | Botón Ver gráficos | Ventana con barras calorías/macros | OK |
| 20 | GUI reporte TXT | Exportar reporte | Archivo `.txt` generado | OK |

|-----|-------------------------|
| Backend | Tablas SQL, `PersonaRepository`, `AlimentoRepository`, inserción y consultas |
| Frontend | Ventana WinAPI, formularios, listas, gráficos, reporte TXT |
| Analista | IMC, calorías, comparación, alertas, archivo `.txt` exportado |

## Consultas SQL de verificación manual

```sql
SELECT * FROM personas;
SELECT * FROM registros_alimenticios WHERE persona_id = 1;
SELECT fecha, SUM(calorias) FROM registros_alimenticios GROUP BY fecha;
```

## Notas

- Los valores nutricionales de referencia siguen lineamientos generales OMS/MSPS.
- El reporte es orientativo; no sustituye valoración médica profesional.
