# =============================================================================
#  geokernel library target
# =============================================================================
find_package(Threads REQUIRED)

add_library(geokernel
  # core — component model, registry, diagnostics
  core/ComponentRegistry.cpp
  core/Log.cpp
  core/Metrics.cpp
  # math
  math/Vec3.cpp
  math/Mat4.cpp
  math/Quat.cpp
  math/Transform.cpp
  # geometry
  geometry/AABB.cpp
  geometry/Mesh.cpp
  # algorithms
  algorithms/MollerTrumboreIntersector.cpp
  algorithms/EdgeFunctionIntersector.cpp
  algorithms/MeshRaycaster.cpp
  # io
  io/detail/ParseUtils.cpp
  io/StlReader.cpp
  io/ObjReader.cpp
  io/MeshLoader.cpp
  # composition root
  Kernel.cpp
)
add_library(GeoKernel::geokernel ALIAS geokernel)

# ---- generated headers -------------------------------------------------------
set(GK_GENERATED_INCLUDE_DIR "${PROJECT_BINARY_DIR}/generated/include")

include(GenerateExportHeader)
generate_export_header(geokernel
  BASE_NAME GK
  EXPORT_FILE_NAME "${GK_GENERATED_INCLUDE_DIR}/geokernel/Export.hpp")

configure_file(
  "${PROJECT_SOURCE_DIR}/cmake/Version.hpp.in"
  "${GK_GENERATED_INCLUDE_DIR}/geokernel/Version.hpp"
  @ONLY)

# ---- usage requirements ------------------------------------------------------
target_compile_features(geokernel PUBLIC cxx_std_17)

target_include_directories(geokernel
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${GK_GENERATED_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})   # private headers: core/LogInternal.hpp, io/detail/*

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(geokernel PUBLIC GK_STATIC_DEFINE)
endif()

# spdlog is compiled into the library (header-only, BUILD_INTERFACE only):
# it never appears in the public API or in the exported link interface.
if(TARGET spdlog::spdlog_header_only)
  target_link_libraries(geokernel PRIVATE $<BUILD_INTERFACE:spdlog::spdlog_header_only>)
else()
  target_link_libraries(geokernel PRIVATE $<BUILD_INTERFACE:spdlog::spdlog>)
endif()

target_link_libraries(geokernel PUBLIC Threads::Threads)

# MinGW/old libstdc++ needed an explicit filesystem library before GCC 9.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
  target_link_libraries(geokernel PUBLIC stdc++fs)
endif()

set_target_properties(geokernel PROPERTIES
  VERSION   ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  OUTPUT_NAME geokernel
  DEBUG_POSTFIX d
  EXPORT_NAME geokernel)

# ---- quality gates -----------------------------------------------------------
gk_set_project_warnings(geokernel)
gk_enable_coverage(geokernel)
gk_enable_clang_tidy(geokernel)
