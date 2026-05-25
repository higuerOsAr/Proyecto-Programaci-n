#include "gui/AppServices.h"
#include "gui/MainWindow.h"

#include "ConsoleMenu.h"
#include "SqliteException.h"

#include <cstdio>
#include <iostream>
#include <windows.h>

static bool wantsConsoleMode() {
    return wcsstr(GetCommandLineW(), L"--console") != nullptr;
}

static int runConsole() {
    AllocConsole();
    FILE* fstdin = stdin;
    FILE* fstdout = stdout;
    FILE* fstderr = stderr;
    freopen_s(&fstdin, "CONIN$", "r", stdin);
    freopen_s(&fstdout, "CONOUT$", "w", stdout);
    freopen_s(&fstderr, "CONOUT$", "w", stderr);

    try {
        AppServices::instance().initialize("nutricion.db");
        auto& app = AppServices::instance();

        std::cout << "Modo consola (--console)\n";
        ConsoleMenu menu(*app.personas, *app.alimentos);
        menu.run();
        return 0;
    } catch (const SqliteException& ex) {
        std::cerr << "Error SQLite (" << ex.code() << "): " << ex.what() << '\n';
        return 1;
    }
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int) {
    if (wantsConsoleMode()) {
        return runConsole();
    }

    try {
        AppServices::instance().initialize("nutricion.db");
        return MainWindow::run(instance);
    } catch (const SqliteException& ex) {
        MessageBoxA(nullptr, ex.what(), "Error SQLite", MB_OK | MB_ICONERROR);
        return 1;
    }
}
