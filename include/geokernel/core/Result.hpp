#pragma once
/// @file Result.hpp
/// @brief Exception-free error propagation for expected failures (I/O, parsing).
///
/// Policy (see docs/adr/0003-error-handling.md):
///  * Expected, recoverable failures (bad file, malformed data) -> Result<T>/Status.
///  * Programming errors (precondition violations)               -> exceptions / asserts.
///  * Hot-path math                                              -> noexcept, no errors.

#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace gk::core {

enum class ErrorCode {
    InvalidArgument,
    NotFound,
    AlreadyExists,
    IoError,
    ParseError,
    UnsupportedFormat,
    LimitExceeded,
    Internal,
};

[[nodiscard]] constexpr std::string_view toString(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::InvalidArgument:   return "InvalidArgument";
        case ErrorCode::NotFound:          return "NotFound";
        case ErrorCode::AlreadyExists:     return "AlreadyExists";
        case ErrorCode::IoError:           return "IoError";
        case ErrorCode::ParseError:        return "ParseError";
        case ErrorCode::UnsupportedFormat: return "UnsupportedFormat";
        case ErrorCode::LimitExceeded:     return "LimitExceeded";
        case ErrorCode::Internal:          return "Internal";
    }
    return "Unknown";
}

struct Error {
    ErrorCode code;
    std::string message;

    [[nodiscard]] std::string toString() const {
        return std::string(gk::core::toString(code)) + ": " + message;
    }
};

/// Thrown only when a caller dereferences a Result that holds an Error —
/// that is a programming error, not an expected failure.
class BadResultAccess : public std::logic_error {
public:
    explicit BadResultAccess(const Error& error)
        : std::logic_error("Result accessed without value -> " + error.toString()) {}
};

/// Either a value of type T or an Error. Minimal subset of C++23 std::expected.
template <typename T>
class [[nodiscard]] Result {
    static_assert(!std::is_same_v<std::decay_t<T>, Error>, "Result<Error> is ambiguous");

public:
    // Separate const&/&& overloads (rather than by-value) so that `return local;`
    // implicitly moves move-only T (e.g. unique_ptr) under C++17 return rules.
    // NOLINTBEGIN(google-explicit-constructor): implicit by design, like std::expected
    Result(const T& value) : storage_(std::in_place_index<0>, value) {}
    Result(T&& value) : storage_(std::in_place_index<0>, std::move(value)) {}
    Result(Error error) : storage_(std::in_place_index<1>, std::move(error)) {}
    // NOLINTEND(google-explicit-constructor)

    [[nodiscard]] bool ok() const noexcept { return storage_.index() == 0; }
    explicit operator bool() const noexcept { return ok(); }

    [[nodiscard]] T& value() & {
        ensureValue();
        return std::get<0>(storage_);
    }
    [[nodiscard]] const T& value() const& {
        ensureValue();
        return std::get<0>(storage_);
    }
    [[nodiscard]] T&& value() && {
        ensureValue();
        return std::get<0>(std::move(storage_));
    }

    [[nodiscard]] const Error& error() const {
        if (ok()) {
            throw std::logic_error("Result::error() called on a successful result");
        }
        return std::get<1>(storage_);
    }

    template <typename U>
    [[nodiscard]] T valueOr(U&& fallback) const& {
        return ok() ? std::get<0>(storage_) : static_cast<T>(std::forward<U>(fallback));
    }

    T* operator->() { return &value(); }
    const T* operator->() const { return &value(); }
    T& operator*() & { return value(); }
    const T& operator*() const& { return value(); }

private:
    void ensureValue() const {
        if (!ok()) {
            throw BadResultAccess(std::get<1>(storage_));
        }
    }

    std::variant<T, Error> storage_;
};

/// Success-or-Error for operations without a return value.
class [[nodiscard]] Status {
public:
    Status() = default;
    // NOLINTNEXTLINE(google-explicit-constructor)
    Status(Error error) : error_(std::move(error)), ok_(false) {}

    [[nodiscard]] static Status success() { return {}; }

    [[nodiscard]] bool ok() const noexcept { return ok_; }
    explicit operator bool() const noexcept { return ok_; }

    [[nodiscard]] const Error& error() const {
        if (ok_) {
            throw std::logic_error("Status::error() called on a successful status");
        }
        return error_;
    }

private:
    Error error_{ErrorCode::Internal, {}};
    bool ok_ = true;
};

/// Convenience factory: `return makeError(ErrorCode::ParseError, "line 3: ...");`
[[nodiscard]] inline Error makeError(ErrorCode code, std::string message) {
    return Error{code, std::move(message)};
}

}  // namespace gk::core
