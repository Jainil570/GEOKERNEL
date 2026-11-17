#include "geokernel/core/Metrics.hpp"

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <utility>

#include "LogInternal.hpp"

namespace gk::metrics {

struct MetricsRegistry::Impl {
    mutable std::shared_mutex mutex;
    // unique_ptr keeps each Counter's address stable across rehash/insert.
    std::map<std::string, std::unique_ptr<Counter>, std::less<>> counters;
};

MetricsRegistry::MetricsRegistry() : impl_(std::make_unique<Impl>()) {}
MetricsRegistry::~MetricsRegistry() = default;

MetricsRegistry& MetricsRegistry::global() {
    static MetricsRegistry instance;
    return instance;
}

Counter& MetricsRegistry::counter(std::string_view name) {
    {
        std::shared_lock lock(impl_->mutex);  // fast path: counter already exists
        if (auto it = impl_->counters.find(name); it != impl_->counters.end()) {
            return *it->second;
        }
    }
    std::unique_lock lock(impl_->mutex);
    auto [it, inserted] = impl_->counters.try_emplace(std::string(name), nullptr);
    if (inserted) {
        it->second = std::make_unique<Counter>();
    }
    return *it->second;
}

void MetricsRegistry::increment(std::string_view name, std::uint64_t delta) {
    counter(name).increment(delta);
}

std::uint64_t MetricsRegistry::value(std::string_view name) const {
    std::shared_lock lock(impl_->mutex);
    const auto it = impl_->counters.find(name);
    return it == impl_->counters.end() ? 0 : it->second->value();
}

std::map<std::string, std::uint64_t> MetricsRegistry::snapshot() const {
    std::shared_lock lock(impl_->mutex);
    std::map<std::string, std::uint64_t> out;
    for (const auto& [name, c] : impl_->counters) {
        out.emplace(name, c->value());
    }
    return out;
}

void MetricsRegistry::reset() {
    std::shared_lock lock(impl_->mutex);  // values are atomic; map is not modified
    for (auto& entry : impl_->counters) {
        entry.second->reset();
    }
}

void MetricsRegistry::logSnapshot() const {
    const auto snap = snapshot();
    GK_LOG_INFO("metrics snapshot ({} counters)", snap.size());
    for (const auto& [name, v] : snap) {
        GK_LOG_INFO("  {:<40} {}", name, v);
    }
}

// ---------------------------------------------------------------------------

ScopedTimer::ScopedTimer(std::string name, MetricsRegistry& registry)
    : name_(std::move(name)), registry_(registry), start_(std::chrono::steady_clock::now()) {}

ScopedTimer::~ScopedTimer() {
    try {
        const auto us = static_cast<std::uint64_t>(elapsed().count());
        registry_.counter(name_ + ".calls").increment();
        registry_.counter(name_ + ".total_us").increment(us);
        registry_.counter(name_ + ".max_us").updateMax(us);
    } catch (...) {  // NOLINT(bugprone-empty-catch): destructors must not throw (bad_alloc)
    }
}

std::chrono::microseconds ScopedTimer::elapsed() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_);
}

}  // namespace gk::metrics
