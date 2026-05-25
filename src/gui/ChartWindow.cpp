#include "ChartWindow.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <string>

namespace {

constexpr wchar_t kChartClass[] = L"NutriChartWindow";
const wchar_t* kChartDataProp = L"NutriChartData";

struct ChartData {
    std::wstring titulo;
    NutritionAnalyzer::AnalisisDia analisis;
};

ChartData* getData(HWND hwnd) {
    return reinterpret_cast<ChartData*>(GetPropW(hwnd, kChartDataProp));
}

void drawBar(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH old = static_cast<HBRUSH>(SelectObject(hdc, brush));
    RECT rc{ x, y, x + w, y + h };
    FillRect(hdc, &rc, brush);
    SelectObject(hdc, old);
    DeleteObject(brush);
}

void drawText(HDC hdc, int x, int y, const wchar_t* text) {
    TextOutW(hdc, x, y, text, static_cast<int>(wcslen(text)));
}

} // namespace

void ChartWindow::show(HWND owner, const std::wstring& titulo,
    const NutritionAnalyzer::AnalisisDia& analisis) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof wc;
        wc.lpfnWndProc = wndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = kChartClass;
        RegisterClassExW(&wc);
        registered = true;
    }

    auto* data = new ChartData{ titulo, analisis };
    HWND hwnd = CreateWindowExW(
        0, kChartClass, L"Grafico nutricional",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 720, 520,
        owner, nullptr, GetModuleHandleW(nullptr), nullptr);

    SetPropW(hwnd, kChartDataProp, data);
    SetWindowTextW(hwnd, titulo.c_str());
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

LRESULT CALLBACK ChartWindow::wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        onPaint(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY: {
        if (auto* data = getData(hwnd)) {
            RemovePropW(hwnd, kChartDataProp);
            delete data;
        }
        return 0;
    }
    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

void ChartWindow::onPaint(HWND hwnd, HDC hdc) {
    const auto* data = getData(hwnd);
    if (!data) {
        return;
    }

    const auto& a = data->analisis;
    RECT client;
    GetClientRect(hwnd, &client);

    drawText(hdc, 20, 15, data->titulo.c_str());

    const int baseY = client.bottom - 60;
    const int chartTop = 80;
    const int chartH = baseY - chartTop;

    // Calorias: consumidas vs recomendadas
    drawText(hdc, 20, 50, L"Calorias (kcal)");
    const double maxCal = (std::max)(a.consumido.calorias,
        (std::max)(a.calorias.requerimientoDiario, 1.0));
    const int barW = 80;
    const int x1 = 40;
    const int x2 = 160;
    const int h1 = static_cast<int>((a.consumido.calorias / maxCal) * chartH);
    const int h2 = static_cast<int>((a.calorias.requerimientoDiario / maxCal) * chartH);
    drawBar(hdc, x1, baseY - h1, barW, h1, RGB(76, 175, 80));
    drawBar(hdc, x2, baseY - h2, barW, h2, RGB(33, 150, 243));
    drawText(hdc, x1, baseY + 8, L"Consumidas");
    drawText(hdc, x2, baseY + 8, L"Recomendadas");

    wchar_t buf[128];
    swprintf_s(buf, L"%.0f vs %.0f kcal", a.consumido.calorias, a.calorias.requerimientoDiario);
    drawText(hdc, 280, 50, buf);

    // Macronutrientes (gramos)
    drawText(hdc, 20, 260, L"Macronutrientes (g)");
    const double maxMacro = (std::max)(a.consumido.carbohidratosG,
        (std::max)(a.consumido.proteinasG,
            (std::max)(a.consumido.grasasG, 1.0)));
    const int mx = 40;
    const int mw = 60;
    const int gap = 90;
    int bx = mx;
    auto macroBar = [&](const wchar_t* label, double val, COLORREF color) {
        const int h = static_cast<int>((val / maxMacro) * 140);
        drawBar(hdc, bx, baseY - h, mw, h, color);
        drawText(hdc, bx, baseY + 8, label);
        bx += gap;
    };
    macroBar(L"Carbs", a.consumido.carbohidratosG, RGB(255, 193, 7));
    macroBar(L"Prot", a.consumido.proteinasG, RGB(156, 39, 176));
    macroBar(L"Grasas", a.consumido.grasasG, RGB(244, 67, 54));

    swprintf_s(buf, L"Azucar: %.1f g | Grasas sat.: %.1f g",
        a.consumido.azucarG, a.consumido.grasasSaturadasG);
    drawText(hdc, 280, 260, buf);
}
