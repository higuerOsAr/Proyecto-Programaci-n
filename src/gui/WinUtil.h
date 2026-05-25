#pragma once

#include <string>
#include <windows.h>

namespace WinUtil {

std::wstring toWide(const std::string& utf8);
std::string toUtf8(const std::wstring& wide);
std::wstring getWindowText(HWND hwnd);
void setWindowText(HWND hwnd, const std::wstring& text);
void messageBox(HWND owner, const std::wstring& text, const std::wstring& title, UINT type = MB_OK);
std::string getEditUtf8(HWND hwnd);
double parseDouble(const std::wstring& text, double fallback = 0.0);
int parseInt(const std::wstring& text, int fallback = 0);

} // namespace WinUtil
