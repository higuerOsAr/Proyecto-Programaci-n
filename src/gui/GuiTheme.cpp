#include "GuiTheme.h"

namespace GuiTheme {

namespace {

HBRUSH g_bg = nullptr;
HBRUSH g_panel = nullptr;
HBRUSH g_edit = nullptr;
HBRUSH g_accent = nullptr;

} // namespace

void init() {
    g_bg = CreateSolidBrush(RGB(225, 242, 230));
    g_panel = CreateSolidBrush(RGB(180, 215, 175));
    g_edit = CreateSolidBrush(RGB(255, 255, 252));
    g_accent = CreateSolidBrush(RGB(56, 142, 96));
}

void cleanup() {
    if (g_bg) DeleteObject(g_bg);
    if (g_panel) DeleteObject(g_panel);
    if (g_edit) DeleteObject(g_edit);
    if (g_accent) DeleteObject(g_accent);
    g_bg = g_panel = g_edit = g_accent = nullptr;
}

HBRUSH background() { return g_bg; }
HBRUSH panel() { return g_panel; }
HBRUSH editBackground() { return g_edit; }
HBRUSH accentButton() { return g_accent; }

COLORREF textPrimary() { return RGB(20, 90, 60); }
COLORREF textHeader() { return RGB(255, 255, 255); }

bool isHeaderControl(int ctrlId) {
    return ctrlId == kHdrPersona || ctrlId == kHdrAlimento || ctrlId == kHdrAcciones;
}

} // namespace GuiTheme
