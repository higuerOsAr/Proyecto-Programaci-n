#pragma once

#include <stdexcept>
#include <string>

class SqliteException : public std::runtime_error {
public:
    explicit SqliteException(const std::string& message, int code = 0)
        : std::runtime_error(message)
        , code_(code) {}

    int code() const noexcept { return code_; }

private:
    int code_;
};
