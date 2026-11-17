#include "geokernel/core/ComponentRegistry.hpp"

#include <algorithm>
#include <exception>
#include <functional>
#include <map>
#include <mutex>
#include <shared_mutex>

#include "LogInternal.hpp"
#include "geokernel/core/Metrics.hpp"

namespace gk::core {

bool ComponentDescriptor::implements(const InterfaceId& iid) const noexcept {
    return std::find(interfaces.begin(), interfaces.end(), iid) != interfaces.end();
}

struct ComponentRegistry::Impl {
    mutable std::shared_mutex mutex;
    std::map<std::string, ComponentDescriptor, std::less<>> components;  // sorted => deterministic
};

ComponentRegistry::ComponentRegistry() : impl_(std::make_unique<Impl>()) {}
ComponentRegistry::~ComponentRegistry() = default;
ComponentRegistry::ComponentRegistry(ComponentRegistry&&) noexcept = default;
ComponentRegistry& ComponentRegistry::operator=(ComponentRegistry&&) noexcept = default;

Status ComponentRegistry::registerComponent(ComponentDescriptor descriptor) {
    if (descriptor.name.empty()) {
        return makeError(ErrorCode::InvalidArgument, "component name must not be empty");
    }
    if (!descriptor.factory) {
        return makeError(ErrorCode::InvalidArgument, "component '" + descriptor.name + "' has no factory");
    }
    if (descriptor.interfaces.empty()) {
        return makeError(ErrorCode::InvalidArgument,
                         "component '" + descriptor.name + "' advertises no interfaces");
    }

    std::unique_lock lock(impl_->mutex);
    const auto name = descriptor.name;
    auto [it, inserted] = impl_->components.try_emplace(name, std::move(descriptor));
    if (!inserted) {
        GK_LOG_WARN("registry: duplicate registration of '{}' rejected", name);
        return makeError(ErrorCode::AlreadyExists, "component '" + name + "' is already registered");
    }
    GK_LOG_DEBUG("registry: registered '{}' v{} ({} interface(s))", name, it->second.version,
                 it->second.interfaces.size());
    metrics::MetricsRegistry::global().increment("registry.registrations");
    return Status::success();
}

bool ComponentRegistry::unregisterComponent(std::string_view name) {
    std::unique_lock lock(impl_->mutex);
    const auto it = impl_->components.find(name);
    if (it == impl_->components.end()) {
        return false;
    }
    impl_->components.erase(it);
    GK_LOG_DEBUG("registry: unregistered '{}'", name);
    return true;
}

bool ComponentRegistry::contains(std::string_view name) const {
    std::shared_lock lock(impl_->mutex);
    return impl_->components.find(name) != impl_->components.end();
}

std::size_t ComponentRegistry::size() const {
    std::shared_lock lock(impl_->mutex);
    return impl_->components.size();
}

bool ComponentRegistry::empty() const {
    return size() == 0;
}

std::vector<std::string> ComponentRegistry::componentNames() const {
    std::shared_lock lock(impl_->mutex);
    std::vector<std::string> names;
    names.reserve(impl_->components.size());
    for (const auto& entry : impl_->components) {
        names.push_back(entry.first);
    }
    return names;  // std::map iteration order => already sorted
}

std::optional<ComponentDescriptor> ComponentRegistry::describe(std::string_view name) const {
    std::shared_lock lock(impl_->mutex);
    const auto it = impl_->components.find(name);
    if (it == impl_->components.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<std::string> ComponentRegistry::findImplementing(const InterfaceId& iid) const {
    std::shared_lock lock(impl_->mutex);
    std::vector<std::string> names;
    for (const auto& [name, descriptor] : impl_->components) {
        if (descriptor.implements(iid)) {
            names.push_back(name);
        }
    }
    return names;
}

std::shared_ptr<IBase> ComponentRegistry::create(std::string_view name) const {
    ComponentFactory factory;
    {
        // Copy the factory and release the lock before invoking it, so a
        // factory may itself use the registry without deadlocking.
        std::shared_lock lock(impl_->mutex);
        const auto it = impl_->components.find(name);
        if (it == impl_->components.end()) {
            GK_LOG_DEBUG("registry: create('{}') -> unknown component", name);
            return nullptr;
        }
        factory = it->second.factory;
    }

    auto& m = metrics::MetricsRegistry::global();
    try {
        auto instance = factory();
        m.increment(instance ? "registry.instances_created" : "registry.factory_null");
        return instance;
    } catch (const std::exception& ex) {
        m.increment("registry.factory_failures");
        GK_LOG_ERROR("registry: factory for '{}' threw: {}", name, ex.what());
    } catch (...) {
        m.increment("registry.factory_failures");
        GK_LOG_ERROR("registry: factory for '{}' threw a non-standard exception", name);
    }
    return nullptr;
}

void ComponentRegistry::clear() {
    std::unique_lock lock(impl_->mutex);
    impl_->components.clear();
}

}  // namespace gk::core
