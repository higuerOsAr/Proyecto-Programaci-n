#pragma once

#include "NutritionAnalyzer.h"

#include <string>
#include <windows.h>

class ChartWindow {
public:
    static void show(HWND owner, const std::wstring& titulo,
        const NutritionAnalyzer::AnalisisDia& analisis);

private:
    static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static void onPaint(HWND hwnd, HDC hdc);
};
