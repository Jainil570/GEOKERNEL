# =============================================================================
#  GeoKernel test suite (GoogleTest + CTest)
# =============================================================================
include(GoogleTest)

add_executable(geokernel_tests
  # core
  unit/core/test_component.cpp
  unit/core/test_result.cpp
  unit/core/test_metrics.cpp
  unit/core/test_log.cpp
  # math
  unit/math/test_vec3.cpp
  unit/math/test_mat4.cpp
  unit/math/test_quat.cpp
  unit/math/test_transform.cpp
)

target_include_directories(geokernel_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src)          # white-box tests of io/detail helpers

target_compile_definitions(geokernel_tests PRIVATE
  GK_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

target_link_libraries(geokernel_tests PRIVATE
  GeoKernel::geokernel
  GTest::gtest
  GTest::gmock
  GTest::gtest_main)

gk_set_project_warnings(geokernel_tests)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # GoogleTest assertion macros expand to casts GCC flags as "useless".
  target_compile_options(geokernel_tests PRIVATE -Wno-useless-cast)
endif()

# One CTest entry per TEST(); DISCOVERY_MODE PRE_TEST avoids running the
# binary at build time (needed for cross-compiling and sanitizer builds).
gtest_discover_tests(geokernel_tests
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DISCOVERY_MODE PRE_TEST
  PROPERTIES
    LABELS "unit"
    ENVIRONMENT "GEOKERNEL_LOG_LEVEL=off")
