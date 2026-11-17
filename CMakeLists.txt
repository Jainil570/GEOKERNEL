# =============================================================================
#  GeoKernel — Component-based C++17 geometry library
# =============================================================================
cmake_minimum_required(VERSION 3.20)

project(GeoKernel
  VERSION 1.0.0
  DESCRIPTION "Component-based C++17 geometry kernel (CAA-style interfaces + plugin registry)"
  HOMEPAGE_URL "https://github.com/Jainil570/GeoKernel"
  LANGUAGES CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not allowed.\n"
    "Use: cmake --preset dev   (or: cmake -S . -B build)")
endif()

# PROJECT_IS_TOP_LEVEL only exists from CMake 3.21.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(GK_IS_TOP_LEVEL ON)
else()
  set(GK_IS_TOP_LEVEL OFF)
endif()

# -----------------------------------------------------------------------------
#  Options
# -----------------------------------------------------------------------------
option(GK_BUILD_TESTS          "Build unit + integration tests"          ${GK_IS_TOP_LEVEL})
option(GK_BUILD_EXAMPLES       "Build example programs"                  ${GK_IS_TOP_LEVEL})
option(GK_BUILD_APPS           "Build the geokernel-cli tool"            ${GK_IS_TOP_LEVEL})
option(GK_BUILD_BENCHMARKS     "Build Google Benchmark micro-benchmarks" OFF)
option(GK_BUILD_DOCS           "Add a 'docs' target (requires Doxygen)"  OFF)
option(GK_ENABLE_COVERAGE      "Instrument for line coverage (GCC/Clang)" OFF)
option(GK_ENABLE_ASAN          "Enable AddressSanitizer"                 OFF)
option(GK_ENABLE_UBSAN         "Enable UndefinedBehaviorSanitizer"       OFF)
option(GK_ENABLE_CLANG_TIDY    "Run clang-tidy on GeoKernel sources"     OFF)
option(GK_WARNINGS_AS_ERRORS   "Treat compiler warnings as errors"       ${GK_IS_TOP_LEVEL})
option(GK_INSTALL              "Generate install / export rules"         ${GK_IS_TOP_LEVEL})
option(BUILD_SHARED_LIBS       "Build GeoKernel as a shared library"     OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # consumed by clang-tidy / IDEs
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CompilerWarnings)
include(Sanitizers)       # must run before dependencies so they are instrumented too
include(Coverage)
include(StaticAnalysis)
include(Dependencies)

# -----------------------------------------------------------------------------
#  Targets
# -----------------------------------------------------------------------------
add_subdirectory(src)

# Must precede every add_subdirectory that calls add_test() (apps/examples
# register smoke tests), otherwise those tests are silently dropped.
if(GK_BUILD_TESTS)
  enable_testing()
endif()

if(GK_BUILD_APPS)
  add_subdirectory(apps)
endif()

if(GK_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(GK_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(GK_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

if(GK_INSTALL)
  include(Install)
  include(Packaging)
endif()

# -----------------------------------------------------------------------------
#  Summary
# -----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "GeoKernel ${PROJECT_VERSION}")
message(STATUS "  Build type ............ ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler .............. ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Shared library ........ ${BUILD_SHARED_LIBS}")
message(STATUS "  Tests / Examples / CLI  ${GK_BUILD_TESTS} / ${GK_BUILD_EXAMPLES} / ${GK_BUILD_APPS}")
message(STATUS "  Benchmarks ............ ${GK_BUILD_BENCHMARKS}")
message(STATUS "  ASan / UBSan .......... ${GK_ENABLE_ASAN} / ${GK_ENABLE_UBSAN}")
message(STATUS "  Coverage .............. ${GK_ENABLE_COVERAGE}")
message(STATUS "  clang-tidy ............ ${GK_ENABLE_CLANG_TIDY}")
message(STATUS "")
