/// Example: extend GeoKernel with a new file format WITHOUT modifying it.
///
/// 1. Implement IMeshReader in a component.
/// 2. Register the component in the registry.
/// 3. MeshLoader (and gk::loadMesh) now open the new extension.
///
/// The ".pts3" format used here is invented for the demo: one triangle per
/// line as nine whitespace-separated numbers.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "geokernel/geokernel.hpp"

namespace demo {

class Pts3Reader final : public gk::core::Component<Pts3Reader, gk::io::IMeshReader> {
public:
    static constexpr std::string_view kComponentName = "demo.Pts3Reader";

    [[nodiscard]] std::string_view formatName() const noexcept override { return "PTS3"; }
    [[nodiscard]] std::vector<std::string> fileExtensions() const override { return {".pts3"}; }

    [[nodiscard]] gk::core::Result<std::unique_ptr<gk::geometry::Mesh>> read(
        std::istream& in, const gk::io::ReadOptions& options) const override {
        auto mesh = std::make_unique<gk::geometry::Mesh>();
        std::string line;
        std::size_t lineNo = 0;
        while (std::getline(in, line)) {
            ++lineNo;
            std::istringstream ss(line);
            double c[9];
            if (!(ss >> c[0] >> c[1] >> c[2] >> c[3] >> c[4] >> c[5] >> c[6] >> c[7] >> c[8])) {
                if (line.find_first_not_of(" \t\r") == std::string::npos) {
                    continue;  // blank line
                }
                return gk::core::makeError(gk::core::ErrorCode::ParseError,
                                           "pts3 line " + std::to_string(lineNo) + ": expected 9 numbers");
            }
            if (mesh->faceCount() >= options.maxTriangles) {
                return gk::core::makeError(gk::core::ErrorCode::LimitExceeded, "too many triangles");
            }
            const auto a = mesh->addVertex({c[0], c[1], c[2]});
            const auto b = mesh->addVertex({c[3], c[4], c[5]});
            const auto d = mesh->addVertex({c[6], c[7], c[8]});
            mesh->addFace(a, b, d);
        }
        return mesh;
    }
};

}  // namespace demo

int main() {
    // Register the plugin into the process-wide registry.
    auto& registry = gk::defaultRegistry();
    if (const auto status = registry.registerComponent<demo::Pts3Reader>("demo .pts3 reader", "0.1.0");
        !status.ok()) {
        std::cerr << status.error().toString() << '\n';
        return 1;
    }

    const auto path = std::filesystem::temp_directory_path() / "geokernel_demo.pts3";
    std::ofstream(path) << "0 0 0  1 0 0  0 1 0\n"
                           "1 0 0  1 1 0  0 1 0\n";

    // Nothing in GeoKernel knows about .pts3 — the registry makes it work.
    auto result = gk::loadMesh(path);
    std::filesystem::remove(path);
    if (!result) {
        std::cerr << "load failed: " << result.error().toString() << '\n';
        return 1;
    }

    std::cout << "loaded " << result.value()->faceCount() << " triangles via plugin, area "
              << result.value()->surfaceArea() << "\nsupported extensions:";
    for (const auto& ext : gk::io::MeshLoader(registry).supportedExtensions()) {
        std::cout << ' ' << ext;
    }
    std::cout << '\n';
    return 0;
}
