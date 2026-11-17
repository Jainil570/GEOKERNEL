#include <gtest/gtest.h>

#include <limits>
#include <sstream>
#include <string>

#include "TestHelpers.hpp"
#include "geokernel/io/StlReader.hpp"

using gk::core::ErrorCode;
using gk::io::ReadOptions;
using gk::io::StlReader;
using gk::math::Vec3;

namespace {

gk::core::Result<std::unique_ptr<gk::geometry::Mesh>> readString(const std::string& bytes,
                                                                 const ReadOptions& opt = {}) {
    std::istringstream in(bytes, std::ios::binary);
    return StlReader{}.read(in, opt);
}

const char* const kAsciiTriangle = R"(solid tri
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
endsolid tri
)";

}  // namespace

TEST(StlReader, ReadsBinaryCubeAndWeldsVertices) {
    const auto bytes = gk::test::makeBinaryStl(gk::test::trianglesOf(gk::test::makeUnitCube()));
    const auto result = readString(bytes);
    ASSERT_TRUE(result.ok()) << result.error().toString();
    const auto& mesh = *result.value();
    EXPECT_EQ(mesh.faceCount(), 12U);
    EXPECT_EQ(mesh.vertexCount(), 8U) << "36 facet vertices weld to 8 unique corners";
    EXPECT_DOUBLE_EQ(mesh.surfaceArea(), 6.0);
}

TEST(StlReader, WeldingCanBeDisabled) {
    const auto bytes = gk::test::makeBinaryStl(gk::test::trianglesOf(gk::test::makeUnitCube()));
    ReadOptions opt;
    opt.weldVertices = false;
    const auto result = readString(bytes, opt);
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value()->vertexCount(), 36U);
}

TEST(StlReader, BinaryHeaderStartingWithSolidIsStillBinary) {
    // Many CAD exporters write "solid" into the binary header: size must win.
    const auto bytes = gk::test::makeBinaryStl({gk::test::makeXYTriangle()}, "solid fake-ascii header");
    const auto result = readString(bytes);
    ASSERT_TRUE(result.ok()) << result.error().toString();
    EXPECT_EQ(result.value()->faceCount(), 1U);
}

TEST(StlReader, ReadsAsciiAndKeepsSolidName) {
    const auto result = readString(kAsciiTriangle);
    ASSERT_TRUE(result.ok()) << result.error().toString();
    EXPECT_EQ(result.value()->faceCount(), 1U);
    EXPECT_EQ(result.value()->name(), "tri");
    EXPECT_EQ(result.value()->triangle(0).normal(), Vec3::unitZ());
}

TEST(StlReader, AsciiParseErrorReportsLineNumber) {
    std::string broken = kAsciiTriangle;
    broken.replace(broken.find("vertex 1 0 0"), 12, "vertex 1 X 0");
    const auto result = readString(broken);
    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, ErrorCode::ParseError);
    EXPECT_NE(result.error().message.find("line 5"), std::string::npos) << result.error().message;
}

TEST(StlReader, AsciiUnexpectedKeywordAndMissingEndsolid) {
    EXPECT_EQ(readString("solid x\n  bogus\nendsolid\n").error().code, ErrorCode::ParseError);

    std::string noEnd = kAsciiTriangle;
    noEnd.erase(noEnd.find("endsolid"));
    const auto lenient = readString(noEnd);
    ASSERT_TRUE(lenient.ok()) << "missing endsolid is tolerated";
    EXPECT_EQ(lenient.value()->faceCount(), 1U);
}

TEST(StlReader, TruncatedBinaryIsRejectedBeforeAllocation) {
    // Header claims 1,000,000 triangles but only one is present.
    const auto bytes = gk::test::makeBinaryStl({gk::test::makeXYTriangle()}, "corrupt", 1'000'000);
    const auto result = readString(bytes);
    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, ErrorCode::ParseError);
    EXPECT_NE(result.error().message.find("truncated"), std::string::npos);
}

TEST(StlReader, TriangleLimitIsEnforced) {
    const auto bytes = gk::test::makeBinaryStl(gk::test::trianglesOf(gk::test::makeUnitCube()));
    ReadOptions opt;
    opt.maxTriangles = 4;
    EXPECT_EQ(readString(bytes, opt).error().code, ErrorCode::LimitExceeded);
}

TEST(StlReader, RejectsNonFiniteCoordinates) {
    const gk::geometry::Triangle bad{{0, 0, 0}, {std::numeric_limits<double>::infinity(), 0, 0}, {0, 1, 0}};
    EXPECT_EQ(readString(gk::test::makeBinaryStl({bad})).error().code, ErrorCode::ParseError);
}

TEST(StlReader, RejectsGarbageAndEmptyInput) {
    EXPECT_EQ(readString("not an stl").error().code, ErrorCode::ParseError);
    EXPECT_EQ(readString("").error().code, ErrorCode::ParseError);
}

TEST(StlReader, AdvertisesFormat) {
    const StlReader reader;
    EXPECT_EQ(reader.formatName(), "STL");
    EXPECT_EQ(reader.fileExtensions(), std::vector<std::string>{".stl"});
}
