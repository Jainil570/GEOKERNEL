#include <gtest/gtest.h>

#include <memory>
#include <string_view>

#include "geokernel/core/Component.hpp"
#include "geokernel/core/IBase.hpp"

namespace {

using gk::core::IBase;
using gk::core::query_interface;

class IGreeter : public virtual IBase {
    GK_DECLARE_INTERFACE("gk.test.IGreeter")
public:
    [[nodiscard]] virtual std::string_view greet() const = 0;
};

class ICounter : public virtual IBase {
    GK_DECLARE_INTERFACE("gk.test.ICounter")
public:
    virtual int next() = 0;
};

class IUnimplemented : public virtual IBase {
    GK_DECLARE_INTERFACE("gk.test.IUnimplemented")
};

/// One component, two interfaces — the CAA "multiple interfaces per object" idiom.
class GreeterCounter final : public gk::core::Component<GreeterCounter, IGreeter, ICounter> {
public:
    static constexpr std::string_view kComponentName = "gk.test.GreeterCounter";
    [[nodiscard]] std::string_view greet() const override { return "hello"; }
    int next() override { return ++count_; }

private:
    int count_ = 0;
};

}  // namespace

TEST(Component, QueryReturnsEachImplementedInterface) {
    std::shared_ptr<IBase> obj = std::make_shared<GreeterCounter>();

    auto greeter = query_interface<IGreeter>(obj);
    auto counter = query_interface<ICounter>(obj);
    ASSERT_NE(greeter, nullptr);
    ASSERT_NE(counter, nullptr);
    EXPECT_EQ(greeter->greet(), "hello");
    EXPECT_EQ(counter->next(), 1);
    EXPECT_EQ(counter->next(), 2);
}

TEST(Component, QueryForUnimplementedInterfaceReturnsNull) {
    std::shared_ptr<IBase> obj = std::make_shared<GreeterCounter>();
    EXPECT_EQ(query_interface<IUnimplemented>(obj), nullptr);
    EXPECT_EQ(query_interface<IGreeter>(std::shared_ptr<IBase>{}), nullptr);
}

TEST(Component, InterfacePointersShareOwnershipWithComponent) {
    std::weak_ptr<IBase> weak;
    std::shared_ptr<ICounter> counter;
    {
        std::shared_ptr<IBase> obj = std::make_shared<GreeterCounter>();
        weak = obj;
        counter = query_interface<ICounter>(obj);
    }
    // The aliasing shared_ptr keeps the whole component alive.
    EXPECT_FALSE(weak.expired());
    counter.reset();
    EXPECT_TRUE(weak.expired());
}

TEST(Component, CrossInterfaceQueryAndIdentity) {
    auto counter = query_interface<ICounter>(std::shared_ptr<IBase>(std::make_shared<GreeterCounter>()));
    auto greeter = query_interface<IGreeter>(counter);  // ICounter -> IGreeter
    ASSERT_NE(greeter, nullptr);

    auto base1 = query_interface<IBase>(counter);
    auto base2 = query_interface<IBase>(greeter);
    EXPECT_EQ(base1.get(), base2.get()) << "one component must expose one IBase identity";
    EXPECT_EQ(base1->componentName(), "gk.test.GreeterCounter");
}

TEST(Component, ProvidedInterfacesListsDeclaredInterfacesInOrder) {
    const auto ids = GreeterCounter::providedInterfaces();
    ASSERT_EQ(ids.size(), 2U);
    EXPECT_EQ(ids[0], IGreeter::iid());
    EXPECT_EQ(ids[1], ICounter::iid());
}
