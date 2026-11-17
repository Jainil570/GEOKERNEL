#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "geokernel/io/ObjReader.hpp"

using gk::core::ErrorCode;
using gk::io::ObjReader;
using gk::io::ReadOptions;
using gk::math::Vec3;

namespace {
gk::core::Result<std::unique_ptr<gk::geometry::Mesh>> readString(const std::string& text,
                                                                 const ReadOptions& opt = {}) {
    std::istringstream in(text);
    return ObjReader{}.read(in, opt);
}
}  // namespace

TEST(ObjReader, ReadsTrianglesWithAllReferenceStyles) {
    const auto result = readString(R"(# comment line
o Widget
v 0 0 0
v 1 0 0
v 0 1 0
v 1 1 0
vt 0 0
vn 0 0 1
f 1 2 3
f 2/1 4/1 3/1
f 1//1 2//1 3//1
f 2/1/1 4/1/1 3/1/1   # trailing comment
)");
    ASSERT_TRUE(result.ok()) << result.error().toString();
    const auto& mesh = *result.value();
    EXPECT_EQ(mesh.name(), "Widget");
    EXPECT_EQ(mesh.vertexCount(), 4U);
    EXPECT_EQ(mesh.faceCount(), 4U);
    EXPECT_EQ(mesh.faces()[1], (gk::geometry::Mesh::Face{1, 3, 2}));
}

TEST(ObjReader, FanTriangulatesPolygons) {
    const auto result = readString("v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nv -1 0.5 0\nf 1 2 3 4 5\n");
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value()->faceCount(), 3U) << "pentagon -> 3 triangles";
    EXPECT_DOUBLE_EQ(result.value()->surfaceArea(), 1.5);
}

TEST(ObjReader, NegativeIndicesAreRelative) {
    const auto result = readString("v 0 0 0\nv 1 0 0\nv 0 1 0\nf -3 -2 -1\n");
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value()->faces()[0], (gk::geometry::Mesh::Face{0, 1, 2}));
}

TEST(ObjReader, ForwardReferencesAreAllowed) {
    const auto result = readString("f 1 2 3\nv 0 0 0\nv 1 0 0\nv 0 1 0\n");
    ASSERT_TRUE(result.ok()) << result.error().toString();
    EXPECT_EQ(result.value()->faceCount(), 1U);
}

TEST(ObjReader, OutOfRangeIndexReportsLine) {
    const auto result = readString("v 0 0 0\nv 1 0 0\nv 0 1 0\n\nf 1 2 9\n");
    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code, ErrorCode::ParseError);
    EXPECT_NE(result.error().message.find("line 5"), std::string::npos) << result.error().message;
}

TEST(ObjReader, MalformedStatementsAreErrors) {
    EXPECT_EQ(readString("v 1 2\n").error().code, ErrorCode::ParseError) << "too few coords";
    EXPECT_EQ(readString("v 1 two 3\n").error().code, ErrorCode::ParseError) << "bad number";
    EXPECT_EQ(readString("v 0 0 0\nf 1 1\n").error().code, ErrorCode::ParseError) << "2-vertex face";
    EXPECT_EQ(readString("v 0 0 0\nf 0 1 1\n").error().code, ErrorCode::ParseError) << "index 0 invalid";
    EXPECT_EQ(readString("v 0 0 0\nf -5 1 1\n").error().code, ErrorCode::ParseError) << "relative underflow";
}

TEST(ObjReader, UnknownStatementsAreIgnoredAndLimitsEnforced) {
    const std::string text =
        "mtllib x.mtl\nusemtl red\ng group\ns 1\ncurv2 1 2\nv 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "f 1 2 3\nf 1 2 3\nf 1 2 3\n";
    ASSERT_TRUE(readString(text).ok());

    ReadOptions opt;
    opt.maxTriangles = 2;
    EXPECT_EQ(readString(text, opt).error().code, ErrorCode::LimitExceeded);
}

TEST(ObjReader, EmptyFileYieldsEmptyMesh) {
    const auto result = readString("# nothing here\n\n");
    ASSERT_TRUE(result.ok());
    EXPECT_TRUE(result.value()->empty());
    EXPECT_EQ(ObjReader{}.formatName(), "OBJ");
}
