#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "geokernel/core/Metrics.hpp"

using gk::metrics::MetricsRegistry;
using gk::metrics::ScopedTimer;

TEST(Metrics, CountersStartAtZeroAndIncrement) {
    MetricsRegistry m;
    EXPECT_EQ(m.value("missing"), 0U);
    m.increment("a");
    m.increment("a", 4);
    EXPECT_EQ(m.value("a"), 5U);
}

TEST(Metrics, CounterHandlesAreStableAcrossInsertAndReset) {
    MetricsRegistry m;
    auto& c = m.counter("stable");
    for (int i = 0; i < 100; ++i) {
        m.increment("filler." + std::to_string(i));  // force map growth
    }
    c.increment(3);
    EXPECT_EQ(m.value("stable"), 3U);

    m.reset();
    EXPECT_EQ(c.value(), 0U) << "reset() zeroes but must not invalidate handles";
    c.increment();
    EXPECT_EQ(m.value("stable"), 1U);
}

TEST(Metrics, SnapshotIsSortedCopy) {
    MetricsRegistry m;
    m.increment("b", 2);
    m.increment("a", 1);
    const auto snap = m.snapshot();
    ASSERT_EQ(snap.size(), 2U);
    EXPECT_EQ(snap.begin()->first, "a");
    EXPECT_EQ(snap.at("b"), 2U);
}

TEST(Metrics, ConcurrentIncrementsAreNotLost) {
    MetricsRegistry m;
    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&m] {
            for (int i = 0; i < 10'000; ++i) {
                m.increment("hot");
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }
    EXPECT_EQ(m.value("hot"), 80'000U);
}

TEST(Metrics, UpdateMaxKeepsLargest) {
    MetricsRegistry m;
    auto& c = m.counter("max");
    c.updateMax(5);
    c.updateMax(3);
    c.updateMax(9);
    EXPECT_EQ(c.value(), 9U);
}

TEST(Metrics, ScopedTimerRecordsCallsAndDuration) {
    MetricsRegistry m;
    {
        ScopedTimer t("op", m);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    { ScopedTimer t("op", m); }
    EXPECT_EQ(m.value("op.calls"), 2U);
    EXPECT_GE(m.value("op.total_us"), 1000U);
    EXPECT_GE(m.value("op.max_us"), 1000U);
    EXPECT_NO_THROW(m.logSnapshot());
}
