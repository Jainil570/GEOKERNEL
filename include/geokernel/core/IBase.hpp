#pragma once
/// @file IBase.hpp
/// @brief Root of every GeoKernel interface (analogous to CAA's CATBaseUnknown).
///
/// Clients never depend on concrete component classes. They hold an
/// `std::shared_ptr<IBase>` (or any interface) and ask for other capabilities
/// through `query_interface<I>()`. Lifetime is shared: an interface pointer
/// obtained by query keeps the whole component alive (aliasing shared_ptr).

#include <memory>
#include <string_view>
#include <type_traits>

#include "geokernel/core/InterfaceId.hpp"

namespace gk::core {

class IBase {
    GK_DECLARE_INTERFACE("gk.core.IBase")

public:
    virtual ~IBase() = default;

    IBase(const IBase&) = delete;
    IBase& operator=(const IBase&) = delete;
    IBase(IBase&&) = delete;
    IBase& operator=(IBase&&) = delete;

    /// Returns a pointer to the requested interface, or nullptr when the
    /// component does not implement it. Never throws. Prefer the typed
    /// `query_interface<I>()` helper over calling this directly.
    [[nodiscard]] virtual void* queryInterface(const InterfaceId& iid) noexcept = 0;

    /// Registry name of the concrete component, e.g. "gk.io.StlReader".
    [[nodiscard]] virtual std::string_view componentName() const noexcept = 0;

protected:
    IBase() = default;
};

/// Typed interface query. The returned pointer shares ownership with `object`.
/// @return nullptr when `object` is null or does not implement `I`.
template <typename I>
[[nodiscard]] std::shared_ptr<I> query_interface(const std::shared_ptr<IBase>& object) noexcept {
    static_assert(std::is_base_of_v<IBase, I>, "query_interface<I>: I must derive from gk::core::IBase");
    if (!object) {
        return nullptr;
    }
    void* raw = object->queryInterface(I::iid());
    if (raw == nullptr) {
        return nullptr;
    }
    return std::shared_ptr<I>(object, static_cast<I*>(raw));  // aliasing constructor
}

/// Cross-interface query: from any interface pointer to any other interface.
template <typename I, typename J,
          typename = std::enable_if_t<!std::is_same_v<J, IBase> && std::is_base_of_v<IBase, J>>>
[[nodiscard]] std::shared_ptr<I> query_interface(const std::shared_ptr<J>& object) noexcept {
    return query_interface<I>(std::static_pointer_cast<IBase>(object));
}

}  // namespace gk::core
