#pragma once

#include <cstdint>
#include <string>

#include <windows.h>

class MainWindow {
public:
    static int run(HINSTANCE instance);

private:
    static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static void onCreate(HWND hwnd);
    static void onCommand(HWND hwnd, int id);
    static void refreshPersonas(HWND hwnd);
    static void refreshAlimentos(HWND hwnd);
    static void updateFechaAlimentoLabel(HWND hwnd);
    static std::int64_t selectedPersonaId(HWND hwnd);
    static std::string fechaHoy();

    static HWND hwnd_;
};
