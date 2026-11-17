# -----------------------------------------------------------------------------
#  Third-party dependencies.
#  Strategy: prefer an installed package (vcpkg / conan / system), otherwise
#  fetch a pinned release with FetchContent. Versions are pinned so builds are
#  reproducible; bump them deliberately via a PR.
# -----------------------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_QUIET ON)

# Treat third-party headers as SYSTEM so our strict -Werror warning set never
# fires inside them (FetchContent_Declare(SYSTEM) needs CMake 3.25).
function(gk_mark_system_includes)
  foreach(target IN LISTS ARGN)
    if(TARGET ${target})
      get_target_property(_aliased ${target} ALIASED_TARGET)
      if(_aliased)
        set(target ${_aliased})
      endif()
      get_target_property(_dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
      if(_dirs)
        set_target_properties(${target} PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_dirs}")
      endif()
    endif()
  endforeach()
endfunction()

set(GK_SPDLOG_VERSION     "1.14.1")
set(GK_GTEST_VERSION      "1.15.2")
set(GK_BENCHMARK_VERSION  "1.9.0")

# ---- spdlog (logging; linked PRIVATE, header-only, never leaks into the API) -
find_package(spdlog ${GK_SPDLOG_VERSION} QUIET CONFIG)
if(NOT spdlog_FOUND)
  message(STATUS "GeoKernel: fetching spdlog v${GK_SPDLOG_VERSION}")
  set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
  set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
  set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v${GK_SPDLOG_VERSION}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(spdlog)
  gk_mark_system_includes(spdlog spdlog_header_only)
  # We consume the header-only target; don't build the compiled lib by default.
  set_target_properties(spdlog PROPERTIES EXCLUDE_FROM_ALL ON)
endif()

# ---- GoogleTest --------------------------------------------------------------
if(GK_BUILD_TESTS)
  find_package(GTest ${GK_GTEST_VERSION} QUIET CONFIG)
  if(NOT GTest_FOUND)
    message(STATUS "GeoKernel: fetching GoogleTest v${GK_GTEST_VERSION}")
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)   # MSVC: match /MD runtime
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v${GK_GTEST_VERSION}.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
    gk_mark_system_includes(gtest gtest_main gmock gmock_main)
  endif()
endif()

# ---- Google Benchmark --------------------------------------------------------
if(GK_BUILD_BENCHMARKS)
  find_package(benchmark ${GK_BENCHMARK_VERSION} QUIET CONFIG)
  if(NOT benchmark_FOUND)
    message(STATUS "GeoKernel: fetching Google Benchmark v${GK_BENCHMARK_VERSION}")
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(benchmark
      URL https://github.com/google/benchmark/archive/refs/tags/v${GK_BENCHMARK_VERSION}.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(benchmark)
    gk_mark_system_includes(benchmark benchmark_main)
  endif()
endif()
