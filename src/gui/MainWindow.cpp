#include "MainWindow.h"

#include "AppServices.h"
#include "ChartWindow.h"
#include "ChimaltenangoData.h"
#include "GuiTheme.h"
#include "WinUtil.h"

#include "NutritionAnalyzer.h"
#include "Persona.h"
#include "RegistroAlimento.h"
#include "ReportGenerator.h"
#include "ReferenciasGuatemala.h"
#include "SqliteException.h"

#include <commctrl.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace {

constexpr wchar_t kMainClass[] = L"NutriMainWindow";
constexpr double kLbPerKg = 2.2046226218;

enum Ctrl : int {
    IDC_NOMBRE = 1001,
    IDC_EDAD,
    IDC_GENERO,
    IDC_MUNICIPIO,
    IDC_PESO_LB,
    IDC_ALTURA,
    IDC_ACTIVIDAD,
    IDC_OCUPACION,
    IDC_META,
    IDC_BTN_GUARDAR_PERSONA,
    IDC_LIST_PERSONAS,
    IDC_PRODUCTO,
    IDC_CANTIDAD,
    IDC_CALORIAS,
    IDC_CARBS,
    IDC_PROTEINAS,
    IDC_GRASAS,
    IDC_AZUCAR,
    IDC_GRASAS_SAT,
    IDC_BTN_AGREGAR_ALIMENTO,
    IDC_LIST_ALIMENTOS,
    IDC_LABEL_FECHA_ALIMENTO,
    IDC_BTN_VER_PERFIL,
    IDC_BTN_ANALISIS,
    IDC_BTN_REPORTE,
    IDC_BTN_GRAFICO,
    IDC_BTN_ELIMINAR_PERSONA,
    IDC_BTN_REINICIAR,
    IDC_BTN_SALIR,
    IDC_EDIT_RESULTADO
};

HWND createLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h) {
    return CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE,
        x, y, w, h, parent, nullptr, nullptr, nullptr);
}

HWND createHeader(HWND parent, int id, const wchar_t* text, int x, int y, int w) {
    return CreateWindowExW(0, L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        x, y, w, 24, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        nullptr, nullptr);
}

HWND createEdit(HWND parent, int id, int x, int y, int w, int h) {
    return CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        x, y, w, h, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        nullptr, nullptr);
}

HWND createButton(HWND parent, int id, const wchar_t* text, int x, int y, int w, int h) {
    return CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, w, h, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        nullptr, nullptr);
}

HWND createCombo(HWND parent, int id, int x, int y, int w, int h) {
    return CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        x, y, w, h, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        nullptr, nullptr);
}

double lbToKg(double lb) { return lb / kLbPerKg; }
double kgToLb(double kg) { return kg * kLbPerKg; }

void fillMunicipiosCombo(HWND combo) {
    for (int i = 0; i < ChimaltenangoData::kNumMunicipios; ++i) {
        SendMessageW(combo, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(ChimaltenangoData::kMunicipios[i]));
    }
    SendMessageW(combo, CB_SETCURSEL, 0, 0);
}

std::string getMunicipioFromCombo(HWND hwnd) {
    HWND combo = GetDlgItem(hwnd, IDC_MUNICIPIO);
    const int sel = static_cast<int>(SendMessageW(combo, CB_GETCURSEL, 0, 0));
    if (sel == CB_ERR) {
        return "Chimaltenango";
    }
    wchar_t buf[128]{};
    SendMessageW(combo, CB_GETLBTEXT, sel, reinterpret_cast<LPARAM>(buf));
    return WinUtil::toUtf8(buf);
}

void clearPersonaForm(HWND hwnd) {
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_NOMBRE), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_EDAD), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_GENERO), L"");
    SendMessageW(GetDlgItem(hwnd, IDC_MUNICIPIO), CB_SETCURSEL, 0, 0);
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_PESO_LB), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_ALTURA), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_ACTIVIDAD), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_OCUPACION), L"");
    SendMessageW(GetDlgItem(hwnd, IDC_META), CB_SETCURSEL, 1, 0);
}

void clearAlimentoForm(HWND hwnd) {
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_PRODUCTO), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_CANTIDAD), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_CALORIAS), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_CARBS), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_PROTEINAS), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_GRASAS), L"");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_AZUCAR), L"0");
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_GRASAS_SAT), L"0");
}

std::string metaFromComboIndex(int index) {
    switch (index) {
    case 0: return "bajar";
    case 2: return "subir";
    default: return "mantener";
    }
}

std::string getMetaFromCombo(HWND hwnd) {
    const int sel = static_cast<int>(SendMessageW(GetDlgItem(hwnd, IDC_META), CB_GETCURSEL, 0, 0));
    if (sel == CB_ERR) {
        return "mantener";
    }
    return metaFromComboIndex(sel);
}

std::wstring formatPerfil(const Persona& p) {
    std::wostringstream o;
    o << std::fixed << std::setprecision(1);
    o << L"=== PERFIL DE PERSONA ===\r\n\r\n";
    o << L"ID: " << p.id << L"\r\n";
    o << L"Nombre: " << WinUtil::toWide(p.nombreCompleto) << L"\r\n";
    o << L"Edad: " << p.edad << L" anios\r\n";
    o << L"Genero: " << WinUtil::toWide(p.genero) << L"\r\n";
    o << L"Departamento: " << WinUtil::toWide(p.departamento) << L" (fijo)\r\n";
    o << L"Municipio: " << WinUtil::toWide(p.municipio) << L"\r\n";
    o << L"Peso: " << kgToLb(p.pesoKg) << L" lb (" << p.pesoKg << L" kg)\r\n";
    o << L"Altura: " << p.alturaCm << L" cm\r\n";
    o << L"Actividad: " << WinUtil::toWide(p.nivelActividad) << L"\r\n";
    o << L"Ocupacion: " << WinUtil::toWide(p.ocupacion) << L"\r\n";
    o << L"Meta: " << WinUtil::toWide(RefGuatemala::etiquetaMeta(p.metaPeso)) << L"\r\n";
    return o.str();
}

LRESULT handleCtlColor(HDC hdc, HWND control) {
    const int id = GetDlgCtrlID(control);

    if (GuiTheme::isHeaderControl(id)) {
        SetBkColor(hdc, RGB(46, 125, 80));
        SetTextColor(hdc, GuiTheme::textHeader());
        return reinterpret_cast<LRESULT>(GuiTheme::accentButton());
    }

    wchar_t className[32]{};
    GetClassNameW(control, className, 32);
    if (wcscmp(className, L"Edit") == 0 || wcscmp(className, L"ListBox") == 0) {
        SetBkColor(hdc, RGB(255, 255, 252));
        SetTextColor(hdc, GuiTheme::textPrimary());
        return reinterpret_cast<LRESULT>(GuiTheme::editBackground());
    }
    if (wcscmp(className, L"Button") == 0) {
        SetBkColor(hdc, RGB(129, 199, 132));
        SetTextColor(hdc, RGB(255, 255, 255));
        return reinterpret_cast<LRESULT>(GuiTheme::panel());
    }

    SetBkColor(hdc, RGB(200, 230, 210));
    SetTextColor(hdc, GuiTheme::textPrimary());
    return reinterpret_cast<LRESULT>(GuiTheme::panel());
}

} // namespace

HWND MainWindow::hwnd_ = nullptr;

int MainWindow::run(HINSTANCE instance) {
    GuiTheme::init();

    INITCOMMONCONTROLSEX icc{ sizeof icc, ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = GuiTheme::background();
    wc.lpszClassName = kMainClass;
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    hwnd_ = CreateWindowExW(
        0, kMainClass,
        L"Nutricion GT - Chimaltenango | UMG 2026",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1120, 740,
        nullptr, nullptr, instance, nullptr);

    if (!hwnd_) {
        GuiTheme::cleanup();
        return 1;
    }

    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    GuiTheme::cleanup();
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK MainWindow::wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        onCreate(hwnd);
        refreshPersonas(hwnd);
        updateFechaAlimentoLabel(hwnd);
        return 0;
    case WM_COMMAND:
        if (HIWORD(wp) == LBN_SELCHANGE && LOWORD(wp) == IDC_LIST_PERSONAS) {
            refreshAlimentos(hwnd);
        } else if (HIWORD(wp) == BN_CLICKED) {
            onCommand(hwnd, LOWORD(wp));
        }
        return 0;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
        return handleCtlColor(reinterpret_cast<HDC>(wp), reinterpret_cast<HWND>(lp));
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

void MainWindow::onCreate(HWND hwnd) {
    createHeader(hwnd, GuiTheme::kHdrPersona, L"  REGISTRO DE PERSONA - CHIMALTENANGO  ", 12, 8, 520);

    createLabel(hwnd, L"Nombre completo:", 15, 40, 120, 18);
    createEdit(hwnd, IDC_NOMBRE, 140, 37, 380, 22);
    createLabel(hwnd, L"Edad:", 15, 67, 50, 18);
    createEdit(hwnd, IDC_EDAD, 70, 64, 55, 22);
    createLabel(hwnd, L"Genero:", 135, 67, 50, 18);
    createEdit(hwnd, IDC_GENERO, 190, 64, 100, 22);

    createLabel(hwnd, L"Departamento:", 15, 94, 95, 18);
    createLabel(hwnd, L"Chimaltenango", 115, 91, 130, 22);
    createLabel(hwnd, L"Municipio:", 260, 94, 70, 18);
    HWND comboMun = createCombo(hwnd, IDC_MUNICIPIO, 335, 91, 185, 200);
    fillMunicipiosCombo(comboMun);

    createLabel(hwnd, L"Peso (lb):", 15, 121, 65, 18);
    createEdit(hwnd, IDC_PESO_LB, 85, 118, 65, 22);
    createLabel(hwnd, L"Altura (cm):", 160, 121, 75, 18);
    createEdit(hwnd, IDC_ALTURA, 240, 118, 60, 22);
    createLabel(hwnd, L"Actividad:", 310, 121, 65, 18);
    createEdit(hwnd, IDC_ACTIVIDAD, 380, 118, 140, 22);

    createLabel(hwnd, L"Ocupacion:", 15, 148, 70, 18);
    createEdit(hwnd, IDC_OCUPACION, 90, 145, 180, 22);
    createLabel(hwnd, L"Meta:", 280, 148, 40, 18);
    HWND comboMeta = createCombo(hwnd, IDC_META, 325, 145, 130, 120);
    SendMessageW(comboMeta, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Bajar de peso"));
    SendMessageW(comboMeta, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Mantener peso"));
    SendMessageW(comboMeta, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Subir de peso"));
    SendMessageW(comboMeta, CB_SETCURSEL, 1, 0);

    createButton(hwnd, IDC_BTN_GUARDAR_PERSONA, L"Guardar", 470, 118, 90, 26);
    createButton(hwnd, IDC_BTN_ELIMINAR_PERSONA, L"Eliminar", 470, 148, 90, 26);
    createButton(hwnd, IDC_BTN_VER_PERFIL, L"Ver perfil", 470, 178, 90, 26);

    createLabel(hwnd, L"Personas registradas:", 15, 182, 180, 18);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
        15, 202, 545, 95, hwnd, reinterpret_cast<HMENU>(IDC_LIST_PERSONAS),
        nullptr, nullptr);

    createHeader(hwnd, GuiTheme::kHdrAlimento, L"  REGISTRO DE ALIMENTOS DEL DIA  ", 12, 305, 520);
    CreateWindowExW(0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE,
        15, 332, 520, 20, hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LABEL_FECHA_ALIMENTO)),
        nullptr, nullptr);

    createLabel(hwnd, L"Producto:", 15, 358, 70, 18);
    createEdit(hwnd, IDC_PRODUCTO, 90, 355, 200, 22);
    createLabel(hwnd, L"Cantidad:", 300, 358, 65, 18);
    createEdit(hwnd, IDC_CANTIDAD, 365, 355, 80, 22);
    createLabel(hwnd, L"Calorias:", 15, 385, 60, 18);
    createEdit(hwnd, IDC_CALORIAS, 75, 382, 55, 22);
    createLabel(hwnd, L"Carbs:", 135, 385, 45, 18);
    createEdit(hwnd, IDC_CARBS, 180, 382, 45, 22);
    createLabel(hwnd, L"Prot:", 230, 385, 35, 18);
    createEdit(hwnd, IDC_PROTEINAS, 265, 382, 45, 22);
    createLabel(hwnd, L"Grasas:", 315, 385, 50, 18);
    createEdit(hwnd, IDC_GRASAS, 365, 382, 45, 22);
    createLabel(hwnd, L"Azucar:", 15, 412, 50, 18);
    createEdit(hwnd, IDC_AZUCAR, 65, 409, 45, 22);
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_AZUCAR), L"0");
    createLabel(hwnd, L"Gras.sat:", 115, 412, 55, 18);
    createEdit(hwnd, IDC_GRASAS_SAT, 170, 409, 45, 22);
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_GRASAS_SAT), L"0");

    createButton(hwnd, IDC_BTN_AGREGAR_ALIMENTO, L"Agregar alimento", 230, 408, 140, 26);

    createLabel(hwnd, L"Alimentos registrados hoy:", 15, 440, 200, 18);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        15, 460, 545, 95, hwnd, reinterpret_cast<HMENU>(IDC_LIST_ALIMENTOS),
        nullptr, nullptr);

    createHeader(hwnd, GuiTheme::kHdrAcciones, L"  ANALISIS Y ACCIONES  ", 580, 8, 500);
    createButton(hwnd, IDC_BTN_ANALISIS, L"Analisis del dia", 590, 45, 160, 32);
    createButton(hwnd, IDC_BTN_REPORTE, L"Exportar reporte TXT", 590, 85, 160, 32);
    createButton(hwnd, IDC_BTN_GRAFICO, L"Ver graficos", 590, 125, 160, 32);
    createButton(hwnd, IDC_BTN_REINICIAR, L"Reiniciar BD", 590, 165, 160, 32);
    createButton(hwnd, IDC_BTN_SALIR, L"Salir", 590, 205, 160, 32);

    createLabel(hwnd, L"Resultado / recomendaciones:", 590, 250, 260, 18);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        590, 270, 490, 285, hwnd, reinterpret_cast<HMENU>(IDC_EDIT_RESULTADO),
        nullptr, nullptr);
}

void MainWindow::updateFechaAlimentoLabel(HWND hwnd) {
    const std::wstring texto = L"Fecha de registro de alimentos: " + WinUtil::toWide(fechaHoy()) +
        L"  (Guatemala)";
    WinUtil::setWindowText(GetDlgItem(hwnd, IDC_LABEL_FECHA_ALIMENTO), texto);
}

std::string MainWindow::fechaHoy() {
    std::time_t t = std::time(nullptr);
    std::tm lt{};
    localtime_s(&lt, &t);
    char buf[16]{};
    std::snprintf(buf, sizeof buf, "%04d-%02d-%02d",
        lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
    return buf;
}

void MainWindow::refreshPersonas(HWND hwnd) {
    HWND list = GetDlgItem(hwnd, IDC_LIST_PERSONAS);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);

    auto& app = AppServices::instance();
    for (const auto& p : app.personas->findAll()) {
        std::wstring line = std::to_wstring(p.id) + L" - " + WinUtil::toWide(p.nombreCompleto)
            + L" (" + WinUtil::toWide(p.municipio) + L")";
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void MainWindow::refreshAlimentos(HWND hwnd) {
    HWND list = GetDlgItem(hwnd, IDC_LIST_ALIMENTOS);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);

    const auto id = selectedPersonaId(hwnd);
    if (id <= 0) {
        return;
    }

    auto& app = AppServices::instance();
    const auto hoy = fechaHoy();
    for (const auto& r : app.alimentos->findByPersonaAndFecha(id, hoy)) {
        std::wostringstream line;
        line << WinUtil::toWide(r.nombreProducto) << L" [" << WinUtil::toWide(r.fecha)
             << L"] " << r.calorias << L" kcal";
        const std::wstring ws = line.str();
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ws.c_str()));
    }
}

std::int64_t MainWindow::selectedPersonaId(HWND hwnd) {
    HWND list = GetDlgItem(hwnd, IDC_LIST_PERSONAS);
    const int sel = static_cast<int>(SendMessageW(list, LB_GETCURSEL, 0, 0));
    if (sel == LB_ERR) {
        return 0;
    }
    wchar_t buf[256]{};
    SendMessageW(list, LB_GETTEXT, sel, reinterpret_cast<LPARAM>(buf));
    return _wtoi64(buf);
}

void MainWindow::onCommand(HWND hwnd, int id) {
    auto& app = AppServices::instance();

    try {
        if (id == IDC_BTN_SALIR) {
            if (MessageBoxW(hwnd, L"Desea cerrar el programa?", L"Salir",
                    MB_YESNO | MB_ICONQUESTION) == IDYES) {
                DestroyWindow(hwnd);
            }
            return;
        }

        if (id == IDC_BTN_GUARDAR_PERSONA) {
            Persona p;
            p.nombreCompleto = WinUtil::getEditUtf8(GetDlgItem(hwnd, IDC_NOMBRE));
            p.edad = WinUtil::parseInt(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_EDAD)));
            p.genero = WinUtil::getEditUtf8(GetDlgItem(hwnd, IDC_GENERO));
            p.departamento = ChimaltenangoData::kDepartamento;
            p.municipio = getMunicipioFromCombo(hwnd);
            const double pesoLb = WinUtil::parseDouble(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_PESO_LB)));
            p.pesoKg = lbToKg(pesoLb);
            p.alturaCm = WinUtil::parseDouble(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_ALTURA)));
            p.nivelActividad = WinUtil::getEditUtf8(GetDlgItem(hwnd, IDC_ACTIVIDAD));
            p.ocupacion = WinUtil::getEditUtf8(GetDlgItem(hwnd, IDC_OCUPACION));
            p.metaPeso = getMetaFromCombo(hwnd);

            const auto newId = app.personas->insert(p);
            WinUtil::messageBox(hwnd, L"Persona guardada ID " + std::to_wstring(newId),
                L"Exito", MB_OK | MB_ICONINFORMATION);
            clearPersonaForm(hwnd);
            refreshPersonas(hwnd);
            return;
        }

        if (id == IDC_BTN_ELIMINAR_PERSONA) {
            const auto personaId = selectedPersonaId(hwnd);
            if (personaId <= 0) {
                WinUtil::messageBox(hwnd, L"Seleccione una persona de la lista.", L"Aviso", MB_OK);
                return;
            }
            if (MessageBoxW(hwnd, L"Se eliminara la persona y todos sus alimentos.",
                    L"Confirmar", MB_YESNO | MB_ICONWARNING) != IDYES) {
                return;
            }
            if (app.personas->remove(personaId)) {
                WinUtil::messageBox(hwnd, L"Persona eliminada.", L"Exito", MB_OK);
                refreshPersonas(hwnd);
                refreshAlimentos(hwnd);
                WinUtil::setWindowText(GetDlgItem(hwnd, IDC_EDIT_RESULTADO), L"");
            } else {
                WinUtil::messageBox(hwnd, L"No se pudo eliminar.", L"Error", MB_OK | MB_ICONERROR);
            }
            return;
        }

        if (id == IDC_BTN_VER_PERFIL) {
            const auto personaId = selectedPersonaId(hwnd);
            if (personaId <= 0) {
                WinUtil::messageBox(hwnd, L"Seleccione una persona de la lista.", L"Aviso", MB_OK);
                return;
            }
            const auto p = app.personas->findById(personaId);
            if (p) {
                WinUtil::setWindowText(GetDlgItem(hwnd, IDC_EDIT_RESULTADO), formatPerfil(*p));
            }
            return;
        }

        if (id == IDC_BTN_AGREGAR_ALIMENTO) {
            const auto personaId = selectedPersonaId(hwnd);
            if (personaId <= 0) {
                WinUtil::messageBox(hwnd, L"Seleccione una persona de la lista.", L"Aviso", MB_OK);
                return;
            }

            updateFechaAlimentoLabel(hwnd);
            const auto fechaRegistro = fechaHoy();

            RegistroAlimento r;
            r.personaId = personaId;
            r.fecha = fechaRegistro;
            r.nombreProducto = WinUtil::getEditUtf8(GetDlgItem(hwnd, IDC_PRODUCTO));
            r.cantidad = WinUtil::getEditUtf8(GetDlgItem(hwnd, IDC_CANTIDAD));
            r.calorias = WinUtil::parseDouble(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_CALORIAS)));
            r.carbohidratosG = WinUtil::parseDouble(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_CARBS)));
            r.proteinasG = WinUtil::parseDouble(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_PROTEINAS)));
            r.grasasG = WinUtil::parseDouble(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_GRASAS)));
            r.azucarG = WinUtil::parseDouble(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_AZUCAR)));
            r.grasasSaturadasG = WinUtil::parseDouble(WinUtil::getWindowText(GetDlgItem(hwnd, IDC_GRASAS_SAT)));

            app.alimentos->insert(r);
            refreshAlimentos(hwnd);
            clearAlimentoForm(hwnd);
            WinUtil::messageBox(hwnd,
                L"Alimento guardado para la fecha " + WinUtil::toWide(fechaRegistro) + L".",
                L"Exito", MB_OK);
            return;
        }

        const auto personaId = selectedPersonaId(hwnd);
        if (personaId <= 0 && id != IDC_BTN_REINICIAR) {
            WinUtil::messageBox(hwnd, L"Seleccione una persona.", L"Aviso", MB_OK);
            return;
        }

        const auto persona = app.personas->findById(personaId);
        if (!persona && id != IDC_BTN_REINICIAR) {
            return;
        }

        const auto hoy = fechaHoy();
        const auto totales = app.alimentos->totalesPorDia(personaId, hoy);
        const auto analisis = app.analyzer.analizarDia(*persona, totales);
        const auto imc = app.analyzer.calcularImc(*persona);

        if (id == IDC_BTN_ANALISIS) {
            std::wstring out = L"=== REFERENCIA GUATEMALA (MSPS) ===\r\n";
            out += WinUtil::toWide(RefGuatemala::kFuenteOficial) + L"\r\n";
            out += WinUtil::toWide(RefGuatemala::kGuiaPlato) + L"\r\n\r\n";

            out += L"IMC: " + std::to_wstring(imc.valor) + L" (" +
                WinUtil::toWide(imc.clasificacion) + L")\r\n";
            out += WinUtil::toWide(imc.recomendacion) + L"\r\n\r\n";

            out += L"Meta: " + WinUtil::toWide(RefGuatemala::etiquetaMeta(persona->metaPeso)) + L"\r\n";
            out += L"Peso registrado: " + std::to_wstring(kgToLb(persona->pesoKg)) + L" lb\r\n";
            out += L"Municipio GT: " + WinUtil::toWide(persona->municipio) + L"\r\n";
            out += L"Calorias consumidas: " + std::to_wstring(static_cast<int>(totales.calorias)) +
                L" kcal\r\n";
            out += L"Calorias recomendadas (GT): " +
                std::to_wstring(static_cast<int>(analisis.calorias.requerimientoDiario)) + L" kcal\r\n";
            out += L"Diferencia: " + std::to_wstring(static_cast<int>(analisis.diferenciaCalorias)) +
                L" kcal\r\n\r\n";

            out += L"ALERTAS:\r\n";
            for (const auto& a : analisis.alertas) {
                out += L"  - " + WinUtil::toWide(a) + L"\r\n";
            }
            out += L"\r\nRECOMENDACIONES:\r\n";
            for (const auto& r : analisis.recomendaciones) {
                out += L"  > " + WinUtil::toWide(r) + L"\r\n";
            }
            for (const auto& alt : app.analyzer.sugerirAlternativas(analisis)) {
                out += L"  * " + WinUtil::toWide(alt) + L"\r\n";
            }
            WinUtil::setWindowText(GetDlgItem(hwnd, IDC_EDIT_RESULTADO), out);
            return;
        }

        if (id == IDC_BTN_REPORTE) {
            const auto lista = app.alimentos->findByPersonaAndFecha(personaId, hoy);
            const auto reporte = ReportGenerator::generarReporteCompleto(
                *persona, hoy, lista, imc, analisis);
            const std::string ruta = "reporte_persona" + std::to_string(personaId) + "_" + hoy + ".txt";
            if (ReportGenerator::guardarEnArchivo(ruta, reporte)) {
                WinUtil::messageBox(hwnd, WinUtil::toWide("Guardado: " + ruta),
                    L"Reporte", MB_OK | MB_ICONINFORMATION);
            }
            return;
        }

        if (id == IDC_BTN_GRAFICO) {
            std::wstring titulo = WinUtil::toWide(persona->nombreCompleto) + L" - " +
                WinUtil::toWide(hoy);
            ChartWindow::show(hwnd, titulo, analisis);
            return;
        }

        if (id == IDC_BTN_REINICIAR) {
            if (MessageBoxW(hwnd, L"Se borraran todos los datos.", L"Confirmar",
                    MB_YESNO | MB_ICONWARNING) == IDYES) {
                app.personas->clearAll();
                refreshPersonas(hwnd);
                refreshAlimentos(hwnd);
                WinUtil::setWindowText(GetDlgItem(hwnd, IDC_EDIT_RESULTADO), L"");
            }
            return;
        }
    } catch (const SqliteException& ex) {
        WinUtil::messageBox(hwnd, WinUtil::toWide(ex.what()), L"Error SQLite", MB_OK | MB_ICONERROR);
    }
}
