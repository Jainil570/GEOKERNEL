#pragma once
/// @file LogInternal.hpp
/// @brief PRIVATE header — the only place GeoKernel sources touch spdlog.
/// Never include this from a public header under include/.

#include <spdlog/spdlog.h>

namespace gk::log::detail {

/// The library-wide logger (not registered in spdlog's global registry, so it
/// never collides with a host application's own "geokernel" logger).
spdlog::logger& logger();

}  // namespace gk::log::detail

#define GK_LOG_TRACE(...) ::gk::log::detail::logger().trace(__VA_ARGS__)
#define GK_LOG_DEBUG(...) ::gk::log::detail::logger().debug(__VA_ARGS__)
#define GK_LOG_INFO(...) ::gk::log::detail::logger().info(__VA_ARGS__)
#define GK_LOG_WARN(...) ::gk::log::detail::logger().warn(__VA_ARGS__)
#define GK_LOG_ERROR(...) ::gk::log::detail::logger().error(__VA_ARGS__)
#define GK_LOG_CRITICAL(...) ::gk::log::detail::logger().critical(__VA_ARGS__)
