#pragma once
/// @file Log.hpp
/// @brief Public logging controls. spdlog is an implementation detail: it is
/// linked privately and never appears in public headers, so consumers are not
/// forced to match our spdlog/fmt versions.
///
/// The initial level is Warn, overridable with the environment variable
/// GEOKERNEL_LOG_LEVEL=trace|debug|info|warn|error|critical|off.

#include <optional>
#include <string>
#include <string_view>

#include "geokernel/Export.hpp"

namespace gk::log {

enum class Level { Trace, Debug, Info, Warn, Error, Critical, Off };

GK_EXPORT void setLevel(Level level);
[[nodiscard]] GK_EXPORT Level level();

/// spdlog pattern syntax, e.g. "[%H:%M:%S.%e] [%^%l%$] %v".
GK_EXPORT void setPattern(const std::string& pattern);

/// Additionally writes log records to `path` (appending). Thread-safe.
/// @return false if the file could not be opened.
GK_EXPORT bool addFileSink(const std::string& path);

/// Writes a record at `lvl` through the GeoKernel logger (for apps/plugins).
GK_EXPORT void write(Level lvl, std::string_view message);

GK_EXPORT void flush();

/// Case-insensitive parse of "trace", "debug", ... "off".
[[nodiscard]] GK_EXPORT std::optional<Level> parseLevel(std::string_view text);
[[nodiscard]] GK_EXPORT std::string_view toString(Level level) noexcept;

}  // namespace gk::log
