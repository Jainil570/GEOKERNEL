/// geokernel-cli — command-line front end for GeoKernel.
///
///   geokernel-cli [global options] <command> [args]
///
/// Commands:
///   info <mesh>                               mesh statistics
///   raycast <mesh> ox oy oz dx dy dz          closest hit of one ray
///   components                                list registered components
///   version                                   print library version
///
/// Global options:
///   --log-level <trace|debug|info|warn|error|off>   (default: warn)
///   --log-file <path>                               also write logs to a file
///   --kernel <component name>                       ray/triangle kernel for raycast
///   --cull                                          ignore back faces in raycast
///   --metrics                                       dump runtime counters on exit
///
/// Exit codes: 0 success, 1 runtime failure, 2 usage error.

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "geokernel/geokernel.hpp"

namespace {

constexpr int kExitOk = 0;
constexpr int kExitFailure = 1;
constexpr int kExitUsage = 2;

struct Options {
    std::string command;
    std::vector<std::string> args;
    std::string kernel = gk::components::kMollerTrumbore;
    bool cull = false;
    bool metrics = false;
};

void printUsage(std::ostream& os) {
    os << "usage: geokernel-cli [--log-level L] [--log-file F] [--kernel K] [--cull] [--metrics]\n"
          "                     <info|raycast|components|version> [args]\n"
          "  info <mesh>\n"
          "  raycast <mesh> ox oy oz dx dy dz\n"
          "  components\n"
          "  version\n";
}

std::optional<Options> parseArgs(int argc, char** argv) {
    Options opt;
    const std::vector<std::string> argsIn(argv + 1, argv + argc);
    for (std::size_t i = 0; i < argsIn.size(); ++i) {
        const std::string& a = argsIn[i];
        const auto needValue = [&]() -> std::optional<std::string> {
            if (i + 1 >= argsIn.size()) {
                std::cerr << "error: " << a << " needs a value\n";
                return std::nullopt;
            }
            return argsIn[++i];
        };

        if (a == "-h" || a == "--help") {
            printUsage(std::cout);
            std::exit(kExitOk);  // NOLINT(concurrency-mt-unsafe)
        } else if (a == "--log-level") {
            const auto v = needValue();
            const auto lvl = v ? gk::log::parseLevel(*v) : std::nullopt;
            if (!lvl) {
                std::cerr << "error: invalid --log-level\n";
                return std::nullopt;
            }
            gk::log::setLevel(*lvl);
        } else if (a == "--log-file") {
            const auto v = needValue();
            if (!v || !gk::log::addFileSink(*v)) {
                std::cerr << "error: cannot open log file\n";
                return std::nullopt;
            }
        } else if (a == "--kernel") {
            const auto v = needValue();
            if (!v) {
                return std::nullopt;
            }
            opt.kernel = *v;
        } else if (a == "--cull") {
            opt.cull = true;
        } else if (a == "--metrics") {
            opt.metrics = true;
        } else if (opt.command.empty()) {
            opt.command = a;
        } else {
            opt.args.push_back(a);
        }
    }
    if (opt.command.empty()) {
        return std::nullopt;
    }
    return opt;
}

std::shared_ptr<gk::geometry::Mesh> loadOrReport(const std::string& path) {
    auto result = gk::loadMesh(path);
    if (!result) {
        std::cerr << "error: " << result.error().toString() << '\n';
        return nullptr;
    }
    return std::move(result).value();
}

int cmdInfo(const Options& opt) {
    if (opt.args.size() != 1) {
        std::cerr << "error: info expects exactly one mesh path\n";
        return kExitUsage;
    }
    const auto mesh = loadOrReport(opt.args[0]);
    if (!mesh) {
        return kExitFailure;
    }
    const auto& b = mesh->bounds();
    std::cout << std::setprecision(6) << "name            : " << mesh->name() << '\n'
              << "vertices        : " << mesh->vertexCount() << '\n'
              << "triangles       : " << mesh->faceCount() << '\n'
              << "degenerate      : " << mesh->countDegenerateFaces() << '\n'
              << "surface area    : " << mesh->surfaceArea() << '\n'
              << "bounds min      : " << b.min() << '\n'
              << "bounds max      : " << b.max() << '\n'
              << "extent          : " << b.extent() << '\n'
              << "memory (bytes)  : " << mesh->memoryFootprint() << '\n';
    return kExitOk;
}

int cmdRaycast(const Options& opt) {
    if (opt.args.size() != 7) {
        std::cerr << "error: raycast expects <mesh> ox oy oz dx dy dz\n";
        return kExitUsage;
    }
    double v[6];
    for (std::size_t i = 0; i < 6; ++i) {
        try {
            std::size_t used = 0;
            v[i] = std::stod(opt.args[i + 1], &used);
            if (used != opt.args[i + 1].size()) {
                throw std::invalid_argument("trailing characters");
            }
        } catch (const std::exception&) {
            std::cerr << "error: invalid number '" << opt.args[i + 1] << "'\n";
            return kExitUsage;
        }
    }

    const auto mesh = loadOrReport(opt.args[0]);
    if (!mesh) {
        return kExitFailure;
    }
    const auto kernel = gk::defaultRegistry().createAs<gk::algorithms::IRayTriangleIntersector>(opt.kernel);
    if (!kernel) {
        std::cerr << "error: unknown kernel '" << opt.kernel << "' (see 'components')\n";
        return kExitUsage;
    }

    const gk::algorithms::MeshRaycaster raycaster(mesh, kernel);
    const gk::geometry::Ray ray{{v[0], v[1], v[2]}, {v[3], v[4], v[5]}};
    const auto cull = opt.cull ? gk::algorithms::CullMode::BackFaces : gk::algorithms::CullMode::None;
    const auto hit = raycaster.closestHit(ray, cull);
    if (!hit) {
        std::cout << "miss\n";
        return kExitOk;
    }
    std::cout << std::setprecision(9) << "hit\n"
              << "  t         : " << hit->t << '\n'
              << "  point     : " << hit->point << '\n'
              << "  normal    : " << hit->normal << '\n'
              << "  face      : " << hit->faceIndex << '\n'
              << "  barycentr.: (" << hit->u << ", " << hit->v << ")\n"
              << "  front face: " << std::boolalpha << hit->frontFace << '\n'
              << "  kernel    : " << raycaster.intersector().componentName() << '\n';
    return kExitOk;
}

int cmdComponents() {
    auto& registry = gk::defaultRegistry();
    for (const auto& name : registry.componentNames()) {
        const auto d = registry.describe(name);
        std::cout << std::left << std::setw(34) << name << " v" << d->version << "  " << d->description
                  << '\n';
        for (const auto& iid : d->interfaces) {
            std::cout << "    implements " << iid.name() << '\n';
        }
    }
    return kExitOk;
}

}  // namespace

int main(int argc, char** argv) {
    const auto opt = parseArgs(argc, argv);
    if (!opt) {
        printUsage(std::cerr);
        return kExitUsage;
    }

    int rc = kExitUsage;
    if (opt->command == "info") {
        rc = cmdInfo(*opt);
    } else if (opt->command == "raycast") {
        rc = cmdRaycast(*opt);
    } else if (opt->command == "components") {
        rc = cmdComponents();
    } else if (opt->command == "version") {
        std::cout << "geokernel " << gk::kVersion.string << '\n';
        rc = kExitOk;
    } else {
        std::cerr << "error: unknown command '" << opt->command << "'\n";
        printUsage(std::cerr);
    }

    if (opt->metrics) {
        for (const auto& [name, value] : gk::metrics::MetricsRegistry::global().snapshot()) {
            std::cout << "[metric] " << name << " = " << value << '\n';
        }
    }
    gk::log::flush();
    return rc;
}
