#pragma once
/// @file Component.hpp
/// @brief CRTP base that implements `queryInterface` for a list of interfaces.
///
/// A concrete component declares the interfaces it provides once:
/// @code
///   class StlReader final : public gk::core::Component<StlReader, IMeshReader> {
///   public:
///       static constexpr std::string_view kComponentName = "gk.io.StlReader";
///       ...
///   };
/// @endcode
/// The interface table is generated at compile time with a fold expression,
/// so there is no hand-written dispatch code to keep in sync.

#include <string_view>
#include <type_traits>
#include <vector>

#include "geokernel/core/IBase.hpp"
#include "geokernel/core/InterfaceId.hpp"

namespace gk::core {

template <typename Derived, typename... Interfaces>
class Component : public Interfaces... {
    static_assert(sizeof...(Interfaces) > 0, "A component must implement at least one interface");
    static_assert((std::is_base_of_v<IBase, Interfaces> && ...),
                  "Every interface must derive (virtually) from gk::core::IBase");

public:
    [[nodiscard]] void* queryInterface(const InterfaceId& iid) noexcept override {
        if (iid == IBase::iid()) {
            return static_cast<IBase*>(this);
        }
        void* result = nullptr;
        // Short-circuiting fold: stops at the first matching interface.
        static_cast<void>(
            ((iid == Interfaces::iid() ? (result = static_cast<Interfaces*>(this), true) : false) || ...));
        return result;
    }

    [[nodiscard]] std::string_view componentName() const noexcept override { return Derived::kComponentName; }

    /// Interfaces advertised in the component registry.
    [[nodiscard]] static std::vector<InterfaceId> providedInterfaces() { return {Interfaces::iid()...}; }

protected:
    Component() = default;
};

}  // namespace gk::core
