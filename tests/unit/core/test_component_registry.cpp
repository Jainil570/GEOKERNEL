#include <gtest/gtest.h>

#include <atomic>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "geokernel/Kernel.hpp"
#include "geokernel/algorithms/IRayTriangleIntersector.hpp"
#include "geokernel/core/Component.hpp"
#include "geokernel/core/ComponentRegistry.hpp"
#include "geokernel/core/Metrics.hpp"
#include "geokernel/io/IMeshReader.hpp"

namespace {

using gk::core::ComponentDescriptor;
using gk::core::ComponentRegistry;
using gk::core::ErrorCode;
using gk::core::IBase;

class IPing : public virtual IBase {
    GK_DECLARE_INTERFACE("gk.test.IPing")
public:
    [[nodiscard]] virtual int ping() const = 0;
};

class PingA final : public gk::core::Component<PingA, IPing> {
public:
    static constexpr std::string_view kComponentName = "test.PingA";
    [[nodiscard]] int ping() const override { return 1; }
};

class PingB final : public gk::core::Component<PingB, IPing> {
public:
    static constexpr std::string_view kComponentName = "test.PingB";
    [[nodiscard]] int ping() const override { return 2; }
};

}  // namespace

TEST(ComponentRegistry, RegisterCreateAndQuery) {
    ComponentRegistry registry;
    ASSERT_TRUE(registry.registerComponent<PingA>("first", "2.1.0").ok());

    EXPECT_TRUE(registry.contains("test.PingA"));
    EXPECT_EQ(registry.size(), 1U);

    auto ping = registry.createAs<IPing>("test.PingA");
    ASSERT_NE(ping, nullptr);
    EXPECT_EQ(ping->ping(), 1);

    const auto info = registry.describe("test.PingA");
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->version, "2.1.0");
    EXPECT_TRUE(info->implements(IPing::iid()));
}

TEST(ComponentRegistry, EachCreateReturnsAFreshInstance) {
    ComponentRegistry registry;
    ASSERT_TRUE(registry.registerComponent<PingA>().ok());
    EXPECT_NE(registry.create("test.PingA").get(), registry.create("test.PingA").get());
}

TEST(ComponentRegistry, RejectsDuplicateNames) {
    ComponentRegistry registry;
    ASSERT_TRUE(registry.registerComponent<PingA>().ok());
    const auto status = registry.registerComponent<PingA>();
    ASSERT_FALSE(status.ok());
    EXPECT_EQ(status.error().code, ErrorCode::AlreadyExists);
}

TEST(ComponentRegistry, RejectsInvalidDescriptors) {
    ComponentRegistry registry;
    ComponentDescriptor noName;
    noName.factory = [] { return std::shared_ptr<IBase>(std::make_shared<PingA>()); };
    noName.interfaces = {IPing::iid()};
    EXPECT_EQ(registry.registerComponent(noName).error().code, ErrorCode::InvalidArgument);

    ComponentDescriptor noFactory;
    noFactory.name = "x";
    noFactory.interfaces = {IPing::iid()};
    EXPECT_EQ(registry.registerComponent(noFactory).error().code, ErrorCode::InvalidArgument);

    ComponentDescriptor noInterfaces;
    noInterfaces.name = "y";
    noInterfaces.factory = noName.factory;
    EXPECT_EQ(registry.registerComponent(noInterfaces).error().code, ErrorCode::InvalidArgument);
    EXPECT_TRUE(registry.empty());
}

TEST(ComponentRegistry, UnknownComponentYieldsNull) {
    ComponentRegistry registry;
    EXPECT_EQ(registry.create("does.not.exist"), nullptr);
    EXPECT_EQ(registry.createAs<IPing>("does.not.exist"), nullptr);
    EXPECT_FALSE(registry.describe("does.not.exist").has_value());
}

TEST(ComponentRegistry, FindImplementingIsSortedAndFiltered) {
    ComponentRegistry registry;
    ASSERT_TRUE(registry.registerComponent<PingB>().ok());
    ASSERT_TRUE(registry.registerComponent<PingA>().ok());

    const auto names = registry.findImplementing(IPing::iid());
    EXPECT_EQ(names, (std::vector<std::string>{"test.PingA", "test.PingB"}));
    EXPECT_TRUE(registry.findImplementing(gk::io::IMeshReader::iid()).empty());
    EXPECT_EQ(registry.createAllImplementing<IPing>().size(), 2U);
}

TEST(ComponentRegistry, UnregisterAndClear) {
    ComponentRegistry registry;
    ASSERT_TRUE(registry.registerComponent<PingA>().ok());
    ASSERT_TRUE(registry.registerComponent<PingB>().ok());

    EXPECT_TRUE(registry.unregisterComponent("test.PingA"));
    EXPECT_FALSE(registry.unregisterComponent("test.PingA"));
    EXPECT_EQ(registry.componentNames(), std::vector<std::string>{"test.PingB"});

    registry.clear();
    EXPECT_TRUE(registry.empty());
}

TEST(ComponentRegistry, ThrowingFactoryIsContainedAndCounted) {
    ComponentRegistry registry;
    ComponentDescriptor d;
    d.name = "test.Throws";
    d.interfaces = {IPing::iid()};
    d.factory = []() -> std::shared_ptr<IBase> { throw std::runtime_error("boom"); };
    ASSERT_TRUE(registry.registerComponent(d).ok());

    auto& metrics = gk::metrics::MetricsRegistry::global();
    const auto before = metrics.value("registry.factory_failures");
    EXPECT_EQ(registry.create("test.Throws"), nullptr);
    EXPECT_EQ(metrics.value("registry.factory_failures"), before + 1);
}

TEST(ComponentRegistry, ConcurrentCreateIsThreadSafe) {
    ComponentRegistry registry;
    ASSERT_TRUE(registry.registerComponent<PingA>().ok());

    std::atomic<int> sum{0};
    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&] {
            for (int i = 0; i < 200; ++i) {
                if (auto p = registry.createAs<IPing>("test.PingA")) {
                    sum += p->ping();
                }
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }
    EXPECT_EQ(sum.load(), 8 * 200);
}

TEST(ComponentRegistry, DefaultRegistryContainsBuiltins) {
    auto& registry = gk::defaultRegistry();
    EXPECT_TRUE(registry.contains(gk::components::kStlReader));
    EXPECT_TRUE(registry.contains(gk::components::kObjReader));
    EXPECT_TRUE(registry.contains(gk::components::kMollerTrumbore));
    EXPECT_TRUE(registry.contains(gk::components::kEdgeFunction));
    EXPECT_EQ(registry.findImplementing(gk::algorithms::IRayTriangleIntersector::iid()).size(), 2U);
    EXPECT_EQ(&registry, &gk::defaultRegistry()) << "must be a singleton";
}

TEST(ComponentRegistry, RegisteringBuiltinsTwiceFails) {
    ComponentRegistry registry;
    ASSERT_TRUE(gk::registerBuiltinComponents(registry).ok());
    EXPECT_EQ(gk::registerBuiltinComponents(registry).error().code, ErrorCode::AlreadyExists);
}
