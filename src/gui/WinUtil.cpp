#include "WinUtil.h"

#include <string>

namespace WinUtil {

std::wstring toWide(const std::string& utf8) {
    if (utf8.empty()) {
        return {};
    }
    const int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (size <= 0) {
        return {};
    }
    std::wstring wide(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), size);
    if (!wide.empty() && wide.back() == L'\0') {
        wide.pop_back();
    }
    return wide;
}

std::string toUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return {};
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }
    std::string utf8(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), size, nullptr, nullptr);
    if (!utf8.empty() && utf8.back() == '\0') {
        utf8.pop_back();
    }
    return utf8;
}

std::wstring getWindowText(HWND hwnd) {
    const int len = GetWindowTextLengthW(hwnd);
    if (len <= 0) {
        return {};
    }
    std::wstring text(static_cast<size_t>(len) + 1, L'\0');
    GetWindowTextW(hwnd, text.data(), len + 1);
    text.resize(static_cast<size_t>(len));
    return text;
}

void setWindowText(HWND hwnd, const std::wstring& text) {
    SetWindowTextW(hwnd, text.c_str());
}

void messageBox(HWND owner, const std::wstring& text, const std::wstring& title, UINT type) {
    MessageBoxW(owner, text.c_str(), title.c_str(), type);
}

std::string getEditUtf8(HWND hwnd) {
    return toUtf8(getWindowText(hwnd));
}

double parseDouble(const std::wstring& text, double fallback) {
    try {
        return std::stod(text);
    } catch (...) {
        return fallback;
    }
}

int parseInt(const std::wstring& text, int fallback) {
    try {
        return std::stoi(text);
    } catch (...) {
        return fallback;
    }
}

} // namespace WinUtil
