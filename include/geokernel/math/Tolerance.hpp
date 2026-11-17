#pragma once
/// @file Tolerance.hpp
/// @brief Kernel-wide numeric tolerances. One source of truth: never sprinkle
/// magic epsilons through algorithms.

#include <algorithm>
#include <cmath>

namespace gk::math {

inline constexpr double kPi = 3.14159265358979323846;

/// Values below this magnitude are treated as numerical zero (determinants,
/// lengths before normalisation, ray/plane parallelism).
inline constexpr double kEpsilon = 1e-12;

/// Default distance tolerance for geometric equality, in model units.
inline constexpr double kLinearTolerance = 1e-9;

/// Default angular tolerance, in radians.
inline constexpr double kAngularTolerance = 1e-9;

[[nodiscard]] constexpr double degToRad(double degrees) noexcept {
    return degrees * kPi / 180.0;
}
[[nodiscard]] constexpr double radToDeg(double radians) noexcept {
    return radians * 180.0 / kPi;
}

/// Mixed absolute/relative comparison: absolute near zero, relative for large values.
[[nodiscard]] inline bool almostEqual(double a, double b, double tolerance = kLinearTolerance) noexcept {
    const double scale = std::max({1.0, std::abs(a), std::abs(b)});
    return std::abs(a - b) <= tolerance * scale;
}

}  // namespace gk::math
