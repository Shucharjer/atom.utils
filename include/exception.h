#pragma once
#include <exception>

namespace atom::utils {

/**
 * @brief An exception class to notify the up layer there is something wrong.
 *
 * This exception means the function has already provided strong grantee, but want to let
 * the caller know.
 */
class notify_exception : public std::exception {
public:
    notify_exception(const char* message) : message_(message) {}

    [[nodiscard]] const char* what() const noexcept override { return message_; }

private:
    const char* message_;
};
} // namespace atom::utils
