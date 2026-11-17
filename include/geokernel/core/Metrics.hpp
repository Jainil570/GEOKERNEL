#pragma once
/// @file Metrics.hpp
/// @brief Lock-free runtime counters + scoped timers, dumpable through the log.
///
/// Counter handles returned by `counter()` have stable addresses for the
/// lifetime of the registry (reset() zeroes values, it never erases), so hot
/// paths can look a counter up once and then increment it lock-free.
/// Convention: counters are aggregated per *operation* (one ray cast, one
/// file load), never per primitive, to keep instrumentation off the hot path.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "geokernel/Export.hpp"

namespace gk::metrics {

class Counter {
public:
    void increment(std::uint64_t delta = 1) noexcept { value_.fetch_add(delta, std::memory_order_relaxed); }
    [[nodiscard]] std::uint64_t value() const noexcept { return value_.load(std::memory_order_relaxed); }
    void reset() noexcept { value_.store(0, std::memory_order_relaxed); }

    /// Atomically raises the stored value to `candidate` if it is larger.
    void updateMax(std::uint64_t candidate) noexcept {
        std::uint64_t current = value_.load(std::memory_order_relaxed);
        while (candidate > current &&
               !value_.compare_exchange_weak(current, candidate, std::memory_order_relaxed)) {
        }
    }

private:
    std::atomic<std::uint64_t> value_{0};
};

class GK_EXPORT MetricsRegistry {
public:
    MetricsRegistry();
    ~MetricsRegistry();
    MetricsRegistry(const MetricsRegistry&) = delete;
    MetricsRegistry& operator=(const MetricsRegistry&) = delete;
    MetricsRegistry(MetricsRegistry&&) = delete;
    MetricsRegistry& operator=(MetricsRegistry&&) = delete;

    /// Process-wide registry used by all built-in components.
    [[nodiscard]] static MetricsRegistry& global();

    /// Returns (creating on first use) the named counter. Thread-safe.
    [[nodiscard]] Counter& counter(std::string_view name);

    void increment(std::string_view name, std::uint64_t delta = 1);

    /// Current value, 0 if the counter has never been created.
    [[nodiscard]] std::uint64_t value(std::string_view name) const;

    /// Consistent-enough copy of every counter (each value read atomically).
    [[nodiscard]] std::map<std::string, std::uint64_t> snapshot() const;

    /// Zeroes all counters; existing Counter& handles remain valid.
    void reset();

    /// Emits every counter at Info level through gk::log.
    void logSnapshot() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/// RAII timer. On destruction records, under `<name>`:
///   <name>.calls     += 1
///   <name>.total_us  += elapsed microseconds
///   <name>.max_us     = max(previous, elapsed)
class GK_EXPORT ScopedTimer {
public:
    explicit ScopedTimer(std::string name, MetricsRegistry& registry = MetricsRegistry::global());
    ~ScopedTimer();
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

    [[nodiscard]] std::chrono::microseconds elapsed() const;

private:
    std::string name_;
    MetricsRegistry& registry_;
    std::chrono::steady_clock::time_point start_;
};

}  // namespace gk::metrics
