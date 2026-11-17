#pragma once
/// @file ParseUtils.hpp
/// @brief PRIVATE parsing helpers shared by the text/binary mesh readers.
/// All functions are locale-independent and never throw on bad input.

#include <cstdint>
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/// Propagates a failed gk::core::Status out of a function returning Result/Status.
#define GK_RETURN_IF_ERROR(expr)       \
    do {                               \
        auto gk_status_ = (expr);      \
        if (!gk_status_.ok()) {        \
            return gk_status_.error(); \
        }                              \
    } while (false)

namespace gk::io::detail {

[[nodiscard]] std::string_view trim(std::string_view text) noexcept;
[[nodiscard]] std::string toLower(std::string_view text);
[[nodiscard]] bool iequals(std::string_view a, std::string_view b) noexcept;

/// Splits on ASCII whitespace; views point into `line`.
void splitWhitespace(std::string_view line, std::vector<std::string_view>& out);

/// Parses the *entire* token as a double ("C" locale, optional leading '+').
[[nodiscard]] bool parseDouble(std::string_view token, double& out) noexcept;

/// Parses the *entire* token as a signed 64-bit integer.
[[nodiscard]] bool parseInt64(std::string_view token, long long& out) noexcept;

/// Bytes from the current read position to the end; restores the position.
/// nullopt for non-seekable streams.
[[nodiscard]] std::optional<std::uint64_t> remainingBytes(std::istream& in);

/// Little-endian decoding independent of host byte order.
[[nodiscard]] std::uint32_t readU32LE(const unsigned char* bytes) noexcept;
[[nodiscard]] float readF32LE(const unsigned char* bytes) noexcept;

/// Whitespace tokenizer over a text stream that tracks line numbers.
/// Returned views stay valid until the next call to next()/restOfLine().
class TokenStream {
public:
    explicit TokenStream(std::istream& in) : in_(in) {}

    /// @return false at end of stream.
    bool next(std::string_view& token);

    /// Remaining (trimmed) text on the current line; moves to the next line.
    [[nodiscard]] std::string restOfLine();

    [[nodiscard]] std::size_t line() const noexcept { return line_; }

private:
    std::istream& in_;
    std::string buffer_;
    std::size_t pos_ = 0;
    std::size_t line_ = 0;
};

}  // namespace gk::io::detail
