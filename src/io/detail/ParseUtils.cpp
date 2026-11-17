#include "ParseUtils.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstring>
#include <limits>
#include <locale>
#include <sstream>

namespace gk::io::detail {
namespace {

bool isSpace(char c) noexcept {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

}  // namespace

std::string_view trim(std::string_view text) noexcept {
    while (!text.empty() && isSpace(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && isSpace(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

std::string toLower(std::string_view text) {
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

bool iequals(std::string_view a, std::string_view b) noexcept {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
               return std::tolower(static_cast<unsigned char>(x)) ==
                      std::tolower(static_cast<unsigned char>(y));
           });
}

void splitWhitespace(std::string_view line, std::vector<std::string_view>& out) {
    out.clear();
    std::size_t i = 0;
    while (i < line.size()) {
        while (i < line.size() && isSpace(line[i])) {
            ++i;
        }
        const std::size_t start = i;
        while (i < line.size() && !isSpace(line[i])) {
            ++i;
        }
        if (i > start) {
            out.push_back(line.substr(start, i - start));
        }
    }
}

bool parseDouble(std::string_view token, double& out) noexcept {
    if (!token.empty() && token.front() == '+') {
        token.remove_prefix(1);
    }
    if (token.empty()) {
        return false;
    }
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    const char* first = token.data();
    const char* last = first + token.size();
    const auto [ptr, ec] = std::from_chars(first, last, out);
    return ec == std::errc{} && ptr == last;
#else
    // Standard libraries without floating-point from_chars (older libc++):
    // a classic-locale stream is slower but equally locale-independent.
    try {
        std::istringstream ss{std::string(token)};
        ss.imbue(std::locale::classic());
        double value = 0.0;
        ss >> value;
        if (ss.fail() || ss.peek() != std::char_traits<char>::eof()) {
            return false;
        }
        out = value;
        return true;
    } catch (...) {
        return false;
    }
#endif
}

bool parseInt64(std::string_view token, long long& out) noexcept {
    if (!token.empty() && token.front() == '+') {
        token.remove_prefix(1);
    }
    const char* first = token.data();
    const char* last = first + token.size();
    const auto [ptr, ec] = std::from_chars(first, last, out);
    return !token.empty() && ec == std::errc{} && ptr == last;
}

std::optional<std::uint64_t> remainingBytes(std::istream& in) {
    const auto here = in.tellg();
    if (here == std::istream::pos_type(-1)) {
        return std::nullopt;
    }
    in.seekg(0, std::ios::end);
    const auto end = in.tellg();
    in.seekg(here);
    if (end == std::istream::pos_type(-1) || end < here) {
        return std::nullopt;
    }
    return static_cast<std::uint64_t>(end - here);
}

std::uint32_t readU32LE(const unsigned char* bytes) noexcept {
    return static_cast<std::uint32_t>(bytes[0]) | (static_cast<std::uint32_t>(bytes[1]) << 8U) |
           (static_cast<std::uint32_t>(bytes[2]) << 16U) | (static_cast<std::uint32_t>(bytes[3]) << 24U);
}

float readF32LE(const unsigned char* bytes) noexcept {
    static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
                  "binary STL requires IEEE-754 single precision floats");
    const std::uint32_t bits = readU32LE(bytes);
    float value = 0.0F;
    std::memcpy(&value, &bits, sizeof value);  // well-defined type punning
    return value;
}

// ---------------------------------------------------------------------------

bool TokenStream::next(std::string_view& token) {
    for (;;) {
        while (pos_ < buffer_.size() && isSpace(buffer_[pos_])) {
            ++pos_;
        }
        if (pos_ < buffer_.size()) {
            const std::size_t start = pos_;
            while (pos_ < buffer_.size() && !isSpace(buffer_[pos_])) {
                ++pos_;
            }
            token = std::string_view(buffer_).substr(start, pos_ - start);
            return true;
        }
        if (!std::getline(in_, buffer_)) {
            return false;
        }
        ++line_;
        pos_ = 0;
    }
}

std::string TokenStream::restOfLine() {
    std::string rest(trim(std::string_view(buffer_).substr(std::min(pos_, buffer_.size()))));
    pos_ = buffer_.size();  // next() will fetch a new line
    return rest;
}

}  // namespace gk::io::detail
