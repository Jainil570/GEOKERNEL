// Links against the *installed* GeoKernel package only (no build-tree paths).
#include <iostream>

#include <geokernel/geokernel.hpp>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: consumer <mesh>\n";
        return 2;
    }
    auto mesh = gk::loadMesh(argv[1]);
    if (!mesh) {
        std::cerr << mesh.error().toString() << '\n';
        return 1;
    }
    std::cout << "GeoKernel " << gk::kVersion.string << ": " << mesh.value()->faceCount() << " triangles\n";
    return mesh.value()->faceCount() > 0 ? 0 : 1;
}
