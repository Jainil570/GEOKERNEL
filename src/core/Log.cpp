#include "geokernel/core/Log.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "LogInternal.hpp"

namespace gk::log {
namespace {

constexpr const char* kDefaultPattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [t:%t] %v";

std::optional<std::string> readEnv(const char* name) {
#if defined(_MSC_VER)
    char* buffer = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&buffer, &length, name) == 0 && buffer != nullptr) {
        std::string value(buffer);
        std::free(buffer);  // NOLINT(cppcoreguidelines-no-malloc): _dupenv_s contract
        return value;
    }
    return std::nullopt;
#else
    const char* value = std::getenv(name);  // NOLINT(concurrency-mt-unsafe): read once at init
    return value != nullptr ? std::optional<std::string>(value) : std::nullopt;
#endif
}

spdlog::level::level_enum toSpd(Level lvl) noexcept {
    switch (lvl) {
        case Level::Trace:    return spdlog::level::trace;
        case Level::Debug:    return spdlog::level::debug;
        case Level::Info:     return spdlog::level::info;
        case Level::Warn:     return spdlog::level::warn;
        case Level::Error:    return spdlog::level::err;
        case Level::Critical: return spdlog::level::critical;
        case Level::Off:      return spdlog::level::off;
    }
    return spdlog::level::warn;
}

Level fromSpd(spdlog::level::level_enum lvl) noexcept {
    switch (lvl) {
        case spdlog::level::trace:    return Level::Trace;
        case spdlog::level::debug:    return Level::Debug;
        case spdlog::level::info:     return Level::Info;
        case spdlog::level::warn:     return Level::Warn;
        case spdlog::level::err:      return Level::Error;
        case spdlog::level::critical: return Level::Critical;
        default:                      return Level::Off;
    }
}

/// Logger + fan-out sink. The dist_sink lets sinks be added at runtime
/// without racing threads that are currently logging.
struct LoggerState {
    std::shared_ptr<spdlog::sinks::dist_sink_mt> fanout;
    std::shared_ptr<spdlog::logger> logger;

    LoggerState() : fanout(std::make_shared<spdlog::sinks::dist_sink_mt>()) {
        fanout->add_sink(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
        logger = std::make_shared<spdlog::logger>("geokernel", fanout);
        logger->set_pattern(kDefaultPattern);
        logger->flush_on(spdlog::level::err);

        Level initial = Level::Warn;
        if (const auto env = readEnv("GEOKERNEL_LOG_LEVEL")) {
            if (const auto parsed = parseLevel(*env)) {
                initial = *parsed;
            }
        }
        logger->set_level(toSpd(initial));
    }
};

LoggerState& state() {
    static LoggerState instance;  // thread-safe initialisation (C++11 magic statics)
    return instance;
}

}  // namespace

namespace detail {
spdlog::logger& logger() {
    return *state().logger;
}
}  // namespace detail

void setLevel(Level lvl) {
    state().logger->set_level(toSpd(lvl));
}

Level level() {
    return fromSpd(state().logger->level());
}

void setPattern(const std::string& pattern) {
    state().logger->set_pattern(pattern);
}

bool addFileSink(const std::string& path) {
    try {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, /*truncate=*/false);
        sink->set_pattern(kDefaultPattern);
        state().fanout->add_sink(std::move(sink));
        return true;
    } catch (const spdlog::spdlog_ex& ex) {
        GK_LOG_ERROR("cannot open log file '{}': {}", path, ex.what());
        return false;
    }
}

void write(Level lvl, std::string_view message) {
    state().logger->log(toSpd(lvl), "{}", message);
}

void flush() {
    state().logger->flush();
}

std::optional<Level> parseLevel(std::string_view text) {
    std::string lower(text);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    static constexpr std::array<std::pair<std::string_view, Level>, 9> kTable{{
        {"trace", Level::Trace},
        {"debug", Level::Debug},
        {"info", Level::Info},
        {"warn", Level::Warn},
        {"warning", Level::Warn},
        {"error", Level::Error},
        {"critical", Level::Critical},
        {"off", Level::Off},
        {"none", Level::Off},
    }};
    for (const auto& [name, lvl] : kTable) {
        if (lower == name) {
            return lvl;
        }
    }
    return std::nullopt;
}

std::string_view toString(Level lvl) noexcept {
    switch (lvl) {
        case Level::Trace:    return "trace";
        case Level::Debug:    return "debug";
        case Level::Info:     return "info";
        case Level::Warn:     return "warn";
        case Level::Error:    return "error";
        case Level::Critical: return "critical";
        case Level::Off:      return "off";
    }
    return "unknown";
}

}  // namespace gk::log
