# -----------------------------------------------------------------------------
#  Install + CMake package export.
#  Consumers use:
#      find_package(GeoKernel 1.0 REQUIRED)
#      target_link_libraries(app PRIVATE GeoKernel::geokernel)
# -----------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(GK_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/GeoKernel")

install(TARGETS geokernel
  EXPORT GeoKernelTargets
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}      COMPONENT runtime
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}      COMPONENT runtime
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}      COMPONENT development
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Public headers (hand-written + generated export/version headers)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT development
  FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY "${PROJECT_BINARY_DIR}/generated/include/"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT development)

install(EXPORT GeoKernelTargets
  FILE GeoKernelTargets.cmake
  NAMESPACE GeoKernel::
  DESTINATION ${GK_CMAKE_INSTALL_DIR}
  COMPONENT development)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/GeoKernelConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/GeoKernelConfig.cmake"
  INSTALL_DESTINATION ${GK_CMAKE_INSTALL_DIR})

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/GeoKernelConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
  "${PROJECT_BINARY_DIR}/GeoKernelConfig.cmake"
  "${PROJECT_BINARY_DIR}/GeoKernelConfigVersion.cmake"
  DESTINATION ${GK_CMAKE_INSTALL_DIR}
  COMPONENT development)

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" "${PROJECT_SOURCE_DIR}/README.md"
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
  COMPONENT development)
