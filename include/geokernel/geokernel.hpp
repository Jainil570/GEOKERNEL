#pragma once
/// @file geokernel.hpp
/// @brief Umbrella header. Convenient for applications; library code and
/// performance-sensitive builds should include only the headers they use.

#include "geokernel/Export.hpp"
#include "geokernel/Kernel.hpp"
#include "geokernel/Version.hpp"

// core — component model, registry, diagnostics
#include "geokernel/core/Component.hpp"
#include "geokernel/core/ComponentRegistry.hpp"
#include "geokernel/core/IBase.hpp"
#include "geokernel/core/InterfaceId.hpp"
#include "geokernel/core/Log.hpp"
#include "geokernel/core/Metrics.hpp"
#include "geokernel/core/Result.hpp"

// math
#include "geokernel/math/Mat4.hpp"
#include "geokernel/math/Quat.hpp"
#include "geokernel/math/Tolerance.hpp"
#include "geokernel/math/Transform.hpp"
#include "geokernel/math/Vec3.hpp"

// geometry
#include "geokernel/geometry/AABB.hpp"
#include "geokernel/geometry/Mesh.hpp"
#include "geokernel/geometry/Ray.hpp"
#include "geokernel/geometry/Triangle.hpp"

// algorithms
#include "geokernel/algorithms/EdgeFunctionIntersector.hpp"
#include "geokernel/algorithms/IRayTriangleIntersector.hpp"
#include "geokernel/algorithms/MeshRaycaster.hpp"
#include "geokernel/algorithms/MollerTrumboreIntersector.hpp"

// io
#include "geokernel/io/IMeshReader.hpp"
#include "geokernel/io/MeshLoader.hpp"
#include "geokernel/io/ObjReader.hpp"
#include "geokernel/io/StlReader.hpp"
