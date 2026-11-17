#include <gtest/gtest.h>

#include "geokernel/algorithms/IRayTriangleIntersector.hpp"
#include "geokernel/core/IBase.hpp"
#include "geokernel/core/InterfaceId.hpp"
#include "geokernel/io/IMeshReader.hpp"

using gk::core::fnv1a64;
using gk::core::InterfaceId;

TEST(InterfaceId, Fnv1aMatchesReferenceVectors) {
    // Published FNV-1a 64-bit test vectors.
    EXPECT_EQ(fnv1a64(""), 0xcbf29ce484222325ULL);
    EXPECT_EQ(fnv1a64("a"), 0xaf63dc4c8601ec8cULL);
    EXPECT_EQ(fnv1a64("foobar"), 0x85944171f73967e8ULL);
}

TEST(InterfaceId, IsUsableAtCompileTime) {
    constexpr InterfaceId id{"gk.test.IFoo"};
    static_assert(id.hash() == fnv1a64("gk.test.IFoo"));
    static_assert(id == InterfaceId{"gk.test.IFoo"});
    static_assert(id != InterfaceId{"gk.test.IBar"});
    EXPECT_EQ(id.name(), "gk.test.IFoo");
}

TEST(InterfaceId, BuiltinInterfacesHaveDistinctIds) {
    const InterfaceId ids[] = {gk::core::IBase::iid(), gk::io::IMeshReader::iid(),
                               gk::algorithms::IRayTriangleIntersector::iid()};
    for (std::size_t i = 0; i < std::size(ids); ++i) {
        for (std::size_t j = i + 1; j < std::size(ids); ++j) {
            EXPECT_NE(ids[i], ids[j]) << ids[i].name() << " vs " << ids[j].name();
        }
    }
}
