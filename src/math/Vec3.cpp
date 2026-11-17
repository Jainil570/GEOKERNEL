#include "geokernel/math/Vec3.hpp"

#include <ostream>

namespace gk::math {

std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    return os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ')';
}

}  // namespace gk::math
