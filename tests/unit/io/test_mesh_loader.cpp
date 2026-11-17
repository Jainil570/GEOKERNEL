#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "TestHelpers.hpp"
#include "geokernel/Kernel.hpp"
#include "geokernel/core/Component.hpp"
#include "geokernel/core/Metrics.hpp"
#include "geokernel/io/MeshLoader.hpp"

using gk::core::ComponentRegistry;
using gk::core::ErrorCode;
using gk::io::IMeshReader;
using gk::io::MeshLoader;
using gk::test::TempFile;

namespace {

/// A third-party "plugin" reader for a trivial format: one triangle per line,
/// nine numbers. Proves new formats plug in without touching MeshLoader.
class TriReader final : public gk::core::Component<TriReader, IMeshReader> {
public:
    static constexpr std::string_view kComponentName = "test.TriReader";
    [[nodiscard]] std::string_view formatName() const noexcept override { return "TRI"; }
    [[nodiscard]] std::vector<std::string> fileExtensions() const override { return {".tri"}; }
    [[nodiscard]] gk::core::Result<std::unique_ptr<gk::geometry::Mesh>> read(
        std::istream& in, const gk::io::ReadOptions& /*options*/) const override {
        auto mesh = std::make_unique<gk::geometry::Mesh>();
        double c[9];
        while (in >> c[0] >> c[1] >> c[2] >> c[3] >> c[4] >> c[5] >> c[6] >> c[7] >> c[8]) {
            const auto a = mesh->addVertex({c[0], c[1], c[2]});
            const auto b = mesh->addVertex({c[3], c[4], c[5]});
            const auto d = mesh->addVertex({c[6], c[7], c[8]});
            mesh->addFace(a, b, d);
        }
        return mesh;
    }
};

}  // namespace

TEST(MeshLoader, LoadsBinaryStlFileAndNamesMeshAfterFile) {
    const TempFile file(".stl", gk::test::makeBinaryStl(gk::test::trianglesOf(gk::test::makeUnitCube())));
    const auto result = gk::loadMesh(file.path());
    ASSERT_TRUE(result.ok()) << result.error().toString();
    EXPECT_EQ(result.value()->faceCount(), 12U);
    EXPECT_EQ(result.value()->name(), file.path().stem().string());
}

TEST(MeshLoader, ExtensionMatchingIsCaseInsensitive) {
    const MeshLoader loader(gk::defaultRegistry());
    EXPECT_NE(loader.findReader(".STL"), nullptr);
    EXPECT_NE(loader.findReader("obj"), nullptr) << "dot is optional";
    EXPECT_EQ(loader.findReader(".xyz"), nullptr);
    EXPECT_EQ(loader.findReader(""), nullptr);
    EXPECT_EQ(loader.supportedExtensions(), (std::vector<std::string>{".obj", ".stl"}));
}

TEST(MeshLoader, ReportsMissingFileAndUnsupportedFormat) {
    auto& m = gk::metrics::MetricsRegistry::global();
    const auto failuresBefore = m.value("io.load_failures");

    EXPECT_EQ(gk::loadMesh("definitely/not/here.stl").error().code, ErrorCode::NotFound);

    const TempFile unknown(".xyz", "data");
    const auto r = gk::loadMesh(unknown.path());
    ASSERT_FALSE(r.ok());
    EXPECT_EQ(r.error().code, ErrorCode::UnsupportedFormat);
    EXPECT_NE(r.error().message.find(".obj, .stl"), std::string::npos) << r.error().message;

    EXPECT_EQ(m.value("io.load_failures"), failuresBefore + 2);
}

TEST(MeshLoader, DirectoryIsRejected) {
    EXPECT_EQ(gk::loadMesh(std::filesystem::temp_directory_path()).error().code, ErrorCode::InvalidArgument);
}

TEST(MeshLoader, StreamOverloadSelectsReaderByExtension) {
    const MeshLoader loader(gk::defaultRegistry());
    std::istringstream obj("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
    const auto result = loader.load(obj, ".obj");
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value()->faceCount(), 1U);

    std::istringstream any("x");
    EXPECT_EQ(loader.load(any, ".nope").error().code, ErrorCode::UnsupportedFormat);
}

TEST(MeshLoader, CustomReaderPluginIsDiscoveredThroughRegistry) {
    ComponentRegistry registry;
    ASSERT_TRUE(gk::registerBuiltinComponents(registry).ok());
    const MeshLoader loader(registry);
    EXPECT_EQ(loader.findReader(".tri"), nullptr);

    ASSERT_TRUE(registry.registerComponent<TriReader>("demo plugin").ok());
    const TempFile file(".tri", "0 0 0  1 0 0  0 1 0\n0 0 1  1 0 1  0 1 1\n");
    const auto result = loader.load(file.path());
    ASSERT_TRUE(result.ok()) << result.error().toString();
    EXPECT_EQ(result.value()->faceCount(), 2U);
    EXPECT_EQ(loader.supportedExtensions(), (std::vector<std::string>{".obj", ".stl", ".tri"}));
}

TEST(MeshLoader, ParseErrorsPropagateFromReader) {
    const TempFile broken(".obj", "v 0 0 0\nf 1 2 3\n");
    const auto r = gk::loadMesh(broken.path());
    ASSERT_FALSE(r.ok());
    EXPECT_EQ(r.error().code, ErrorCode::ParseError);
}
