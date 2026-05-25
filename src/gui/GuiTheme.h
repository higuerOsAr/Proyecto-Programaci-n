#pragma once

#include <windows.h>

namespace GuiTheme {

inline constexpr int kHdrPersona = 2001;
inline constexpr int kHdrAlimento = 2002;
inline constexpr int kHdrAcciones = 2003;

void init();
void cleanup();

HBRUSH background();
HBRUSH panel();
HBRUSH editBackground();
HBRUSH accentButton();

COLORREF textPrimary();
COLORREF textHeader();

bool isHeaderControl(int ctrlId);

} // namespace GuiTheme
