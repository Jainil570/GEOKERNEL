/// Example: load a mesh file, inspect it, transform it.
///   example_load_and_inspect path/to/mesh.{stl,obj}

#include <iostream>

#include "geokernel/geokernel.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <mesh.stl|mesh.obj>\n";
        return 2;
    }

    gk::log::setLevel(gk::log::Level::Info);  // show the loader's timing line

    auto result = gk::loadMesh(argv[1]);
    if (!result) {
        std::cerr << "failed: " << result.error().toString() << '\n';
        return 1;
    }
    std::unique_ptr<gk::geometry::Mesh> mesh = std::move(result).value();

    std::cout << "mesh '" << mesh->name() << "': " << mesh->vertexCount() << " vertices, "
              << mesh->faceCount() << " triangles, area " << mesh->surfaceArea() << '\n'
              << "bounds " << mesh->bounds().min() << " .. " << mesh->bounds().max() << '\n';

    // Place the mesh: scale x2, rotate 90 deg about Z, then move up 10 units.
    const auto placement =
        gk::math::Transform::uniformScaling(2.0)
            .then(gk::math::Transform::rotation(gk::math::Vec3::unitZ(), gk::math::degToRad(90)))
            .then(gk::math::Transform::translation({0, 0, 10}));
    mesh->transform(placement);

    std::cout << "after transform: bounds " << mesh->bounds().min() << " .. " << mesh->bounds().max()
              << ", area " << mesh->surfaceArea() << '\n';
    return 0;
}
