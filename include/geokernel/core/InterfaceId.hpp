#pragma once
/// @file InterfaceId.hpp
/// @brief Stable, compile-time interface identifiers (the CAA "IID" concept).
///
/// Every public interface declares a globally unique, human-readable name
/// (e.g. "gk.io.IMeshReader"). The name is hashed at compile time with
/// FNV-1a so identity checks are a single integer compare in the common case,
/// with a full string compare to rule out hash collisions.

#include <cstdint>
#include <string_view>

namespace gk::core {

/// 64-bit FNV-1a hash, usable in constant expressions.
[[nodiscard]] constexpr std::uint64_t fnv1a64(std::string_view text) noexcept {
    std::uint64_t hash = 14695981039346656037ULL;
    for (const char c : text) {
        hash ^= static_cast<std::uint8_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

/// Identifies an interface independently of RTTI and of the module (DLL/SO)
/// that defines it. Cheap to copy; compares by hash then by name.
class InterfaceId {
public:
    constexpr explicit InterfaceId(std::string_view name) noexcept : name_(name), hash_(fnv1a64(name)) {}

    [[nodiscard]] constexpr std::string_view name() const noexcept { return name_; }
    [[nodiscard]] constexpr std::uint64_t hash() const noexcept { return hash_; }

    friend constexpr bool operator==(const InterfaceId& lhs, const InterfaceId& rhs) noexcept {
        return lhs.hash_ == rhs.hash_ && lhs.name_ == rhs.name_;
    }
    friend constexpr bool operator!=(const InterfaceId& lhs, const InterfaceId& rhs) noexcept {
        return !(lhs == rhs);
    }

private:
    std::string_view name_;
    std::uint64_t hash_;
};

}  // namespace gk::core

/// Declares the static interface identifier inside an interface class body.
/// Usage:  class IFoo : public virtual gk::core::IBase { GK_DECLARE_INTERFACE("gk.x.IFoo") ... };
#define GK_DECLARE_INTERFACE(QualifiedName)                                 \
public:                                                                     \
    [[nodiscard]] static constexpr ::gk::core::InterfaceId iid() noexcept { \
        return ::gk::core::InterfaceId{QualifiedName};                      \
    }                                                                       \
                                                                            \
private:
