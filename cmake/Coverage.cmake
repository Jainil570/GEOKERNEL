# -----------------------------------------------------------------------------
#  gk_enable_coverage(<target>)
#  Instruments a target for gcov-compatible line/branch coverage. The flags
#  are PUBLIC so test executables linking the library get the runtime too.
#  Reports are produced by scripts/coverage.sh (gcovr) — CI enforces >= 85 %.
# -----------------------------------------------------------------------------
function(gk_enable_coverage target)
  if(NOT GK_ENABLE_COVERAGE)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PUBLIC --coverage -O0 -g -fno-inline)
    target_link_options(${target} PUBLIC --coverage)
  else()
    message(WARNING "GeoKernel: coverage not supported for ${CMAKE_CXX_COMPILER_ID}")
  endif()
endfunction()
