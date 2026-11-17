#pragma once
/// @file ComponentRegistry.hpp
/// @brief Thread-safe plugin registry: name -> factory + advertised interfaces.
///
/// The registry is the only place that knows concrete component types.
/// Everything else asks it for "something that implements IMeshReader"
/// (late binding), which is what lets new formats or algorithms be plugged
/// in without touching client code.

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "geokernel/Export.hpp"
#include "geokernel/core/IBase.hpp"
#include "geokernel/core/InterfaceId.hpp"
#include "geokernel/core/Result.hpp"

namespace gk::core {

using ComponentFactory = std::function<std::shared_ptr<IBase>()>;

/// Static metadata + factory describing one registrable component.
struct ComponentDescriptor {
    std::string name;                     ///< Unique key, e.g. "gk.io.StlReader"
    std::string description;              ///< One-line, human readable
    std::string version = "1.0.0";        ///< Semantic version of the implementation
    std::vector<InterfaceId> interfaces;  ///< Interfaces the component advertises
    ComponentFactory factory;             ///< Creates a fresh instance

    [[nodiscard]] bool implements(const InterfaceId& iid) const noexcept;
};

class GK_EXPORT ComponentRegistry {
public:
    ComponentRegistry();
    ~ComponentRegistry();

    ComponentRegistry(const ComponentRegistry&) = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;
    ComponentRegistry(ComponentRegistry&&) noexcept;
    ComponentRegistry& operator=(ComponentRegistry&&) noexcept;

    /// Registers a component. Fails with AlreadyExists on a duplicate name and
    /// with InvalidArgument when the name, factory or interface list is empty.
    Status registerComponent(ComponentDescriptor descriptor);

    /// Registers a component type `C` that derives from gk::core::Component.
    template <typename C>
    Status registerComponent(std::string description = {}, std::string version = "1.0.0") {
        ComponentDescriptor d;
        d.name = std::string(C::kComponentName);
        d.description = std::move(description);
        d.version = std::move(version);
        d.interfaces = C::providedInterfaces();
        d.factory = [] { return std::static_pointer_cast<IBase>(std::make_shared<C>()); };
        return registerComponent(std::move(d));
    }

    /// @return true if the component existed and was removed.
    bool unregisterComponent(std::string_view name);

    [[nodiscard]] bool contains(std::string_view name) const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] bool empty() const;

    /// Sorted list of every registered component name.
    [[nodiscard]] std::vector<std::string> componentNames() const;

    /// Metadata for a component (factory included), or nullopt.
    [[nodiscard]] std::optional<ComponentDescriptor> describe(std::string_view name) const;

    /// Names of all components advertising `iid`, sorted.
    [[nodiscard]] std::vector<std::string> findImplementing(const InterfaceId& iid) const;

    /// Instantiates a component by name. nullptr when unknown or the factory
    /// throws (the failure is logged and counted).
    [[nodiscard]] std::shared_ptr<IBase> create(std::string_view name) const;

    /// Instantiates and queries in one step.
    template <typename I>
    [[nodiscard]] std::shared_ptr<I> createAs(std::string_view name) const {
        return query_interface<I>(create(name));
    }

    /// Instantiates one instance of every component that implements `I`.
    template <typename I>
    [[nodiscard]] std::vector<std::shared_ptr<I>> createAllImplementing() const {
        std::vector<std::shared_ptr<I>> out;
        for (const auto& name : findImplementing(I::iid())) {
            if (auto instance = createAs<I>(name)) {
                out.push_back(std::move(instance));
            }
        }
        return out;
    }

    void clear();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace gk::core
